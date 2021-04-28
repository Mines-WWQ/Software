//////// Include necessary libraries for sensors

#include <DallasTemperature.h>
#include <Sodaq_DS3231.h>
#include <OneWire.h>
#include <DS18B20.h>
//#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>

/////////////////       Define sensors location on board

// Data wire is connented to the Arduino digital pin B10
#define ONE_WIRE_BUS B10

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

#define DS18S20_Pin D10; //DS18S20 Signal pin on digital 10

//Temperature chip i/o
OneWire ds(B10);  // on digital pin 10

///  Add more sensor locations here


//////////////////     Setup for data log drive and file and sample rate

// Define pin for MicroSD card 
const int8_t SdSsPin = 12;
SdFat SD;

//The data log file name
const char *fileName = "DataLog.txt";

// Data log file and header  
// Change file name to relevant information of recording site/ implementation
// Change header for implemented sensors and data recorded
const char *loggerName = "Mayfly microSD Card Tester";
const char *dataHeader = "SampleNumber, Temperature, Turb_Voltage, pH_Val";

// pH sensor definitions
#define SensorPin A2            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

/// Define data log sample rate (milliseconds, converted to seconds in loop)
int sampleinterval = 15;
int samplenum = 1;

//////////////////       Function for setting up log file
void setupLogFile()
{
  //Initialise the SD card
  if (!SD.begin(SdSsPin))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
  }

  //Check if the file already exists
  bool oldFile = SD.exists(fileName);

  //Open the file in write mode
  File logFile = SD.open(fileName, FILE_WRITE);

  //Add header information if the file did not already exist
  if (!oldFile)
  {
    logFile.println(loggerName);
    logFile.println(dataHeader);
  }

  //Close the file to save it
  logFile.close();
}

////////////////        Logging data to file 
void logData(String rec)
{
  //Re-open the file
  File logFile = SD.open(fileName, FILE_WRITE);

  //Write the CSV data
  logFile.println(rec);

  //Close the file to save it
  logFile.close();
}

/////////////// Send off data to cloud via LTE, function
///// Currently not implemented due to lack of hardware

/////////////// Record GPS location, function
///// Currently not implemented due to lack of hardware 

/////////////// Measure Dissolved Oxygen, function
///// Currently not implemented due to lack of hardware

 
////////////////   Function for measuring Digital Temperature 
//// Currently not recording correctly, catches first exception;
//// potentially due to a missing hardware piece, needs more testing
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      Serial.println("Not addr?");
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;

}

///////////////// Subfunction for pH measurement, ph_volt_val()
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
} 

/////////////////// Function for pH value
/// Gets analog meassurement from pH sensor and calculates pH value from voltage
float ph_volt_val()
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:");
        Serial.print(voltage,2);
        Serial.print("    pH value: ");
    Serial.println(pHValue,2);
        digitalWrite(LED,digitalRead(LED)^1);
        printTime=millis();
  }
  return pHValue;
}

///// Add additional sensor functions here




////////////////////// Function creating data log 
/// Creates string of the sensor data in form:
/// Data num, Temperature, Turbidity Voltage, pH value, *and more*, Time of data recording
String createDataRecord()
{
  //Create a String type data record in csv format
  //SampleNumber, Temp, Volt_turb, pH value
  String data = "";
  data += samplenum;        // creates a string called "data", put in the sample number
  data += ",";   
  data += getTemp();        // Start Temperature measurement
  data += ",";  
  int sensorValue = analogRead(A0);// read the input on analog pin 0:
  float voltage = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  data += voltage;          // Voltage for turbitity
  data += ","; 
  float phval,ph_volt;      // Start pH measurement
  phval = ph_volt_val();
  data += phval;
  
  // Add more sensors and data info here

  /// Record real time clock
  // Not currently developed or implemented
  
  samplenum++;   //increment the sample number
  return data;
}


///////////// Startup function
// put your setup code here, to run once on startup
void setup() {

  //Initialise the serial connection
  Serial.begin(57600);

  //Initialise log file
  setupLogFile();

  //Echo the data header to the serial connection
  Serial.println(dataHeader);
  
  pinMode(22, OUTPUT);    // Setting up Pin 22 to provide power to Grove Ports
  digitalWrite(22, HIGH); // Provide power to D10-11 and D6-7 Grove Ports

  sensors.begin();


  /// implement other sensors startup needs here
  // not currently developed or implemented
  

  int startupdelay=30;   // initial delay on startup to "warm up" sensors
  delay(startupdelay*1000);  // currently set at 30 seconds
}


////////////  Continous function loop, put your main code here to run repeatedly and call other functions
void loop() {
  
  // Create data record
  String dataRec = createDataRecord();

  // Save the data record to the log file
  logData(dataRec);

  // Echo the data to the serial connection
  Serial.println(dataRec);

  /// Include LTE connection for data send off here, if implemented, once all data has been recorded for all sensors
  // not currently developed or implemented


  /// Include exception to end loop and power off device if data storage is near full
  // not currently developed or implemented


  /// Include exception to pause data recording if power supply is critically low
  // not currently developed or implemented  


  /// Time Delay between data record, how often you want data to be collected
  // Minimum delays vary with individual sensors and some sensors take longer than others 
  delay(sampleinterval*1000);   // multiply by 1000 to convert from milliseconds to seconds

}
