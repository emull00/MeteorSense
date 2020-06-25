#include <Wire.h>  // include wire communciations library for ADC
#include <MCP342x.h> // inlude ADC library for mag
#include <MMA_7455.h> //Include the MMA_7455 library for tilt
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
//                    ** GND
//                    ** Vcc 3.3 V ###IMPORTANT### 
//                    ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
//                    ** MOSI - pin 11
//                    ** CLK - pin 13
//                    ** MISO - pin 12

//
//
// startup components
//
//



// -------------------------- set up the SD card drive parameters--------------------------------
const int chipSelect = 4;  // indicates open comms. to SD 

// -------------------------- set up the tilt / accel. params--------------------------------
uint8_t tilt_address = 0x1D; //address for tilt is at 0x1D, make sure two addresses are different.
MMA_7455 mySensor = MMA_7455(); //Make an instance of MMA_7455 

// -------------------------- set up the ADC parameters--------------------------------
uint8_t adc_address = 0x6E; // 0x68 is the default address for all MCP342x devices

MCP342x adc = MCP342x(adc_address);

// Resolution is 12, 14, 16,or 18; gain is 1, 2, 4, or 8.
MCP342x::Config config(MCP342x::channel1, MCP342x::oneShot,MCP342x::resolution18, MCP342x::gain1);
MCP342x::Config status;// Configuration/status read back from the ADC


//
//
//  Setup
//
//



void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Wire.begin();

//+++++++++++++++++++++++  SET up ADC  ++++++++++++++++++++ 
  
 // Check ADC present
  Wire.requestFrom(adc_address, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No adc device found at address ");
    Serial.println(adc_address, HEX);
    while (1);
  } else {Serial.println("Found ADC");}

 ///+++++++++++++++++++++++  SET up TILT sensor  ++++++++++++++++++++ 
  mySensor.initSensitivity(2); // set up  for "Tilt" measurements: 2 = 2g, 4 = 4g, 8 = 8g
    
  // Check TILT present
  Wire.requestFrom(tilt_address, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No tilt device found at address ");
    Serial.println(tilt_address, HEX);
    while (1);
  }else {Serial.println("Found TILT");}

///+++++++++++++++++++++++  SET up SD card drive ++++++++++++++++++++ 

  while (!Serial) {;}  // wait for serial port to connect. Needed for native USB port only
}

void loop() {
   char gpsString[85];          // container for output string
   char lat[14],lon[14];        // string container for lat, long, type double
   uint8_t dummyRead;           // container for ADC call response
   long magX, magY, magZ;       // container for Mag readings.
  
// Get magnetometer, x, y, z
     dummyRead = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
              MCP342x::resolution18, MCP342x::gain1,10, magX, status);
     dummyRead = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
              MCP342x::resolution18, MCP342x::gain1,10, magY, status);
     dummyRead = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
              MCP342x::resolution18, MCP342x::gain1,10, magZ, status);
              
     
     //format yy mm dd hr:mn:sc:mss ----------lat ----------lon Tix Tiy Tiz --------MX --------MY --------MZ
     sprintf(gpsString, "%5l %5l %5l %5d %5d %5d",
     magX, magY, magZ,                                                                 // Magnetometer.
     mySensor.readAxis('x'), mySensor.readAxis('y'), mySensor.readAxis('z'));            // read & add in the tilt
        
      Serial.println(gpsString);

      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.println(gpsString);
      dataFile.close();    

}
