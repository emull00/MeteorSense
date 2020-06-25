#include <Wire.h>
#include <MCP342x.h>


// 0x68 is the default address for all MCP342x devices
uint8_t address = 0x6E;
MCP342x adc = MCP342x(address);

// Configuration settings
// Resolution is 12, 14, 16,or 18; gain is 1, 2, 4, or 8.
MCP342x::Config config(MCP342x::channel1, MCP342x::oneShot,
           MCP342x::resolution16, MCP342x::gain1);

// Configuration/status read back from the ADC
MCP342x::Config status;
// Inidicate if a new conversion should be started
bool startConversion = false;




void setup(void)
{
  Serial.begin(9600);
  Wire.begin();

    
  // Reset devices
  MCP342x::generalCallReset();
  delay(1); // MC342x needs 300us to settle
  
  // Check device present
  Wire.requestFrom(address, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No device found at address ");
    Serial.println(address, HEX);
    while (1);
  }

  // First time loop() is called start a conversion
  startConversion = true;
  Serial.println("X,Y,Z");  // just a line to set up header info for plot
}

unsigned long lastLedFlash = 0;
void loop(void)
{
  long valueX = 0;
  long valueY = 0;
  long valueZ = 0;
  long mag = 0;
  uint8_t err;
  uint8_t readX;
  uint8_t readY;
  uint8_t readZ;

  if (startConversion) {
//    Serial.println("Convert");
    err = adc.convert(config);
    if (err) {
      Serial.print("Convert error: ");
      Serial.println(err);
    }
    startConversion = false;
  }
  readX = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
           MCP342x::resolution16, MCP342x::gain1,10, valueX, status);
  readY = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
           MCP342x::resolution16, MCP342x::gain1,10, valueY, status);
  readZ = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
           MCP342x::resolution16, MCP342x::gain1,10, valueZ, status);
           
  if (!readX && !readY && !readZ && status.isReady()) { 
    // For debugging purposes print the return value.
    mag = sqrt(pow(valueX,2)+pow(valueY,2)+pow(valueZ,2));

    Serial.print(valueX);Serial.print(",");
    Serial.print(valueY);Serial.print(",");
    Serial.print(valueZ); Serial.print(",");
    Serial.print(mag);Serial.print(",");
    Serial.println();
startConversion = true;
  }
    
}
