/*
My Youtube Channel  : http://www.youtube.com/user/Shadow5549
Complete build source code related files can be found here:  http://www.thingiverse.com/thing:252267
This is SD card datalogger designed to supportthe High Altitude Ballon Project which can be followed on my YouTube videos

Videos will be produced throughout the development of this project.  Each stage will be 
doccumented seprately and shared in order (hopefully)

From Arduino Example Code:
This example shows how to log data from analog sensors to an SD card using the SD library.	
The circuit:
 * analog sensors on analog ins 0, 1, 
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (digital assigned via setup)
 ** VCC & Ground
 -------------------------------------------------------------------
 V2- 
 Adding RTC support
 RTC Connected to VCC and Ground
 I2C Connections:
 RTC SDA connected to pin Analog 4
 RTC SCL connected to pin Analog 5

Used I2CScanner.ino to find I2C addresses- 0x50 (EEPROM?)  and 0x68 found
Used SetRTC.ino to manually set proper date/date on RTC module- This wil set the date/time as soon as serial 
 is initiated so set ahead by a minute and wait till time matches then open serial window to set.
----------------------------------------------------------------------- 
V3-
Added Adafruit 992- MPL115A2 Baro/Temp Sensor 5volt 12C device
SDA connected to pin Analog 4
SCL connected to pin Analog 5
Code below should be optimized to avoid multiple I2C calls

Added Battery Voltage Monitoring:
Changed A0 to be used for Bus Voltage monitoring (on board LiPo)
Will use voltage divider with 2 10Kohm resistores to monitor the 7.4V supply from battery
http://forum.arduino.cc/index.php/topic,13728.0.html
Testing- Ran solid 15 hours no issues, 47000 lines recorded, Filesize 2.1MB
-----------------------------------------------------------------------
V4
Added output for altitude in meters (Thanks Jon)
http://forum.arduino.cc/index.php?topic=63726.0
Used static value of 15 for Temp as using actual temp causes unreliable values indoors 

------------------------------------------------------------------------
V5
Added code for LM35 Temp sensor (Will be internal monitoring) on A1 data2 and temp variables
Had to do 2 analog reads to debounce the values with a small delay.
Added Bluetooth- shortened serial write text to fit phone screen
W
----------------------------------------------------------------------
*/
#include <Adafruit_MPL115A2.h>// Support for Baro Sensor Adafruit 992- MPL115A2
Adafruit_MPL115A2 mpl115a2;//Support for Baro Sensor Adafruit 992- MPL115A2
#include "Wire.h"
#define DS1307_ADDRESS 0x68//RTC Support
//Eric Note- another address showed up in scan at 0X50 EEPROM?
byte zero = 0x00; //workaround for issue #527 RTC Support

const float referenceVolts = 5.0;        // the default reference on a 5 volt board 
const int batteryPin = 0;          // +V from battery is connected to analog pin 0
const float sea_press = 1013.25;//added for altitude calc
float alt=0;//added for altitude calc

#include <SD.h>
File myFile; //SD
const int chipSelect=4; // set chipselect pin to 4 for SD
//int dat1=A0;//set data pin
int dat2=1;//define pin0 connect with LM35
float temp=0;
//int data1=0;
int data2=0;
int i=0;
boolean present=0;// SD Card available & usable
void setup()
{
  
  Wire.begin(); 
  Serial.begin(9600);
  mpl115a2.begin();// Baro Sensor Adafruit 992- MPL115A2
  //pinMode(dat1,INPUT);
  pinMode(dat2,INPUT);
  checkSD();
  // Write column headers for Excel CSV
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile)
  {
    //Prints headers to file
    myFile.print("Time"); // Prints Date and Time header
    myFile.print(",");
    myFile.print("line#");// prints line number Header for troubleshooting etc
    myFile.print(",");
    myFile.print("BusVoltage");
    myFile.print(",");
    myFile.print("IntTemp");
    myFile.print(",");
    myFile.print("Pressure");
    myFile.print(",");
    myFile.print("ExtTemp");
    myFile.print(",");
    myFile.println("Alt(m)");
    myFile.close();
    
  }
}
void loop()
{
  int val = analogRead(batteryPin);  // read the value from the A0 battery monitoring pin with voltage divider
  float volts = (val / 511.0) * referenceVolts ; // divide val by (1023/2)because the resistors divide the voltage in half
  
  float pressureKPA = 0, temperatureC = 0; //Adafruit 992- MPL115A2
  if(present==1)
  {
    pressureKPA = mpl115a2.getPressure();//get pressure Adafruit 992- MPL115A2
    temperatureC = mpl115a2.getTemperature(); //Adafruit 992- MPL115A2
    printDate(); //Runs the print date subroutine below
    i++;
    //data1=analogRead(dat1);
  analogRead(1);// added to take initial read to stabilize the ADC
  delay(10);// Delay to stabilize the ADC
  temp = analogRead(1) * 5000L / 1024L  / 10;// Temperature calculation formula
    //data2=analogRead(dat2); //used for Ext Temp sensor TBD
    //temp= data2 * 4.9 / 10 ; // Temperature calculation formula
    Serial.print(" BusV:");
    Serial.print(volts);
    Serial.print(" ITemp:");
    Serial.print(temp);
     Serial.print(" Press:");
    Serial.print(pressureKPA, 4);
    Serial.print(" ExTemp:");
    Serial.print(temperatureC, 1);
    alt= ((pow((sea_press / (pressureKPA *10)), 1/5.257) - 1.0) * (15 + 273.15)) / 0.0065;//rough altitude equation in meters can use real temp instead of 15
    Serial.print(" Alt:");
    Serial.println(alt);
    writeSD();
  }
  delay(1000);
  }

byte decToBcd(byte val){     //Eric-      needed for RTC variable stuff
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)  {    //Eric-      needed for RTC variable stuff
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}



void checkSD()
{
  Serial.print("check SD card");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed");
    // don't do anything more:
    return;
  }
  Serial.println("SD card OK");
  present=1;
  delay(2000);
}


void writeSD()
{
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile)
  {
    int val = analogRead(batteryPin);  // read the value from the A0 battery monitoring pin with voltage divider
  float volts = (val / 511.0) * referenceVolts ; // divide val by (1023/2)because the resistors divide the voltage in half
    // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());
float pressureKPA = mpl115a2.getPressure();//get pressure Adafruit 992- MPL115A2- these are redundant calls to I2c but I dont want to fix
float temperatureC = mpl115a2.getTemperature();//get temp Adafruit 992- MPL115A2- these are redundant calls to I2c but I dont want to fix

  //Print RTC values to SD
  myFile.print(month);
  myFile.print("/");
  myFile.print(monthDay);
  myFile.print("/");
  myFile.print(year);
  myFile.print(" ");
  myFile.print(hour);
  myFile.print(":");
  myFile.print(minute);
  myFile.print(":");
  myFile.print(second);
  //myFile.print(":");
  myFile.print(",");
  //End Print RTC values
    myFile.print(i);
    myFile.print(",");
    myFile.print(volts);
    myFile.print(",");
    myFile.print(temp);
    myFile.print(",");
    myFile.print(pressureKPA, 4);
    myFile.print(",");
    myFile.print(temperatureC, 1);
    myFile.print(",");
    myFile.println(alt);
    myFile.close();
    
  }
}


  void printDate()// Only used to print to serial and not needed for logging purposes
  {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());

  //print the date EG   3/1/11 23:59:59
  Serial.print(month);
  Serial.print("/");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);

}


