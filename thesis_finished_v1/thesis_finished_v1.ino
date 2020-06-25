 
#include <NMEAGPS.h>
#include <NeoSWSerial.h>
#include <Wire.h>  // include wire communciations library
#include <MCP342x.h> // inlude ADC library 
#include <MMA_7455.h> //Include the MMA_7455 library
#include <SPI.h> // SPI library (for SD card)
#include <SD.h>  // SD card read library




// onboard units
// SDCARD
// OLED
// TILT
// ADC - > FGM
// GPS

// -- SDA, SCL, gnd, 5v (lower module ADC, OLED, TILT )
// -- gnd, 5v, SDA, SCL ->  OLED
// -- gnd, 7, 8, 5V  -> GPS
// -- 3.3v, 4, 11,12,13, gnd-> SDCARD


// -------------------------- set up the SD card drive parameters--------------------------------
// ** GND
// ** Vcc 3.3 V ###IMPORTANT### 
// ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
// ** MOSI - pin 11
// ** CLK - pin 13
// ** MISO - pin 12
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 4;  // this is used to signal comms. to SD 

// -------------------------- set up LED panel params-ns
const int SDStatus = 5;
const int GPSStatus = 3;  
const int SysGoStatus = 2;
const int SysGoSwitch = 6;
// -------------------------- set up the GPS parameters--------------------------------
NeoSWSerial gpsPort(7, 8); ////GPS RX= Ard pin 8, GPS TX=Ard pin 7 
NMEAGPS GPS;
bool initial = true;

// -------------------------- set up the ADC parameters--------------------------------
uint8_t adc_address = 0x6E; // 0x68 is the default address for all MCP342x devices
MCP342x adc = MCP342x(adc_address);

// Resolution is 12, 14, 16,or 18; gain is 1, 2, 4, or 8.
MCP342x::Config config(MCP342x::channel1, MCP342x::oneShot,MCP342x::resolution16, MCP342x::gain1);
MCP342x::Config status;// Configuration/status read back from the ADC

// -------------------------- set up the tilt / accel. params--------------------------------
uint8_t tilt_address = 0x1D; //address for tilt is at 0x1D, make sure two addresses are different.
MMA_7455 mySensor = MMA_7455(); //Make an instance of MMA_7455 NEED THIS FOR TILT

void setup()
{
  Serial.begin(115200);
  gpsPort.begin(9600);
  Wire.begin();

///+++++++++++++++++++++++  SET up TILT sensor  ++++++++++++++++++++ 
  mySensor.initSensitivity(2); // set up  for "Tilt" measurements: 2 = 2g, 4 = 4g, 8 = 8g

//+++++++++++++++++++++++  SET up LED panel  ++++++++++++++++++++ 
  pinMode(SDStatus, OUTPUT);
  pinMode(SysGoStatus, OUTPUT);
  pinMode(SysGoSwitch, INPUT);
  pinMode(GPSStatus, OUTPUT);
  
    digitalWrite(SDStatus, HIGH);
    digitalWrite(SysGoStatus, HIGH);
    digitalWrite(GPSStatus, HIGH);

 ///+++++++++++++++++++++++  SET up SD card drive ++++++++++++++++++++ 


  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
        while(1){                                               // go into error display
          digitalWrite(SDStatus, !digitalRead(SDStatus));       // set to flashing
          delay(250);
        }
    while (1);
  }else {
      SD.remove("datalog.txt");
      delay(1000); // wait 1 sec for system to settle.
      digitalWrite(SDStatus, LOW);     // indicator panel shows ok

      }

 ///+++++++++++++++++++++++  Set up here is finished, set up GPS in main loop++++++++++++++++++++ 


}

  
void loop() {

 long magX, magY, magZ;
 String gpsString = "";
 String magString = "";
 String tiltString = "";
 uint8_t dummyRead;

  if(initial){                                     // If starting, assume no GPS yet. start flashing status LED
      digitalWrite(GPSStatus, !digitalRead(GPSStatus));
      delay(50);                                         
    }
 
  if (GPS.available( gpsPort )) {
    gps_fix fix = GPS.read();
    if (fix.valid.location) {
    digitalWrite(GPSStatus, LOW);                  // found a satellite, write to status pin

    if(initial){                                   // Block here and wait for operator okay signal.       
      while(digitalRead(SysGoSwitch));
      digitalWrite(SysGoStatus, LOW);
      delay(200);
      initial = false;
    }

    // Extract readings from all three devices, and output as a single String
    // This is not space-efficient, but this is more readable than character strings.

    // Firstly, go get reading from GPS, incl. time and date
       String gpsString = String(fix.dateTime.year) + ":" + String(fix.dateTime.month) + ":" + String(fix.dateTime.date)+":" ;
        gpsString += String(fix.dateTime.hours) + ":" + String(fix.dateTime.minutes) + ":" +String(fix.dateTime.seconds);
        gpsString += "." + String(fix.dateTime_ms())+" ";
        gpsString += String(fix.latitude(),9) + ":" + String(fix.longitude(),9)+" ";        

    // Next, get tilt info
        char tiltX = mySensor.readAxis('x'); //Read out the 'x' Axis
        char tiltY = mySensor.readAxis('y'); //Read out the 'y' Axis
        char tiltZ = mySensor.readAxis('z'); //Read out the 'z' Axis
        gpsString += String(tiltX,DEC) + " " + String(tiltY,DEC) + " " + String(tiltZ,DEC) + " ";


    // Finally, get magnetometer from ADC. 
        dummyRead = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                 MCP342x::resolution14, MCP342x::gain1,10, magX, status);
        dummyRead = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                 MCP342x::resolution14, MCP342x::gain1,10, magY, status);
        dummyRead = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                 MCP342x::resolution14, MCP342x::gain1,10, magZ, status);
        gpsString += String(magX)+ " " + String(magY) + " " + String(magZ)+" ";
      
  // Write it to file   
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.println(gpsString);
      dataFile.close();    
      
      }                                                             // fix.valid.location
    }                                                               // Gps port
  // If operator turns off data taking (push button), Block here.
    if(!digitalRead(SysGoSwitch)){
       digitalWrite(SysGoStatus, HIGH);
      delay(500);
      while(digitalRead(SysGoSwitch));
      delay(500);
      digitalWrite(SysGoStatus, HIGH);
    }
 
}
