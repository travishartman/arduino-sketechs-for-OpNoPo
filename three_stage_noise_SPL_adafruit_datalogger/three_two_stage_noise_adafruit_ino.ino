/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
// A simple data logger for the Arduino analog pins 
 
// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  50 // mills between entries (reduce to take more/faster data)
 
 // how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()


// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3

// The analog pins that connect to the sensors
#define soundPin 0           // analog 0
//#define tempPin 1                // analog 1
//#define BANDGAPREF 14            // special indicator that we want to measure the bandgap

//#define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!
//#define bandgap_voltage 1.1      // this is not super guaranteed but its not -too- off
 
 
 RTC_DS1307 RTC; // define the Real Time Clock object
 
 

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
 
 
// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
}



 
 
const int sampleWindowS = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sampleS;
const int sampleWindowF = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sampleF;
const int sampleWindowF = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sampleSPL;
 
 
// ##################################################
void setup(void)
{
  Serial.begin(9600);
  Serial.println();
  
  ///tells ifRTC is running
//  if (! RTC.isrunning()) {
//    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL


  }
  

  logfile.println("datetime, sound level");    
#if ECHO_TO_SERIAL
  Serial.println("datetime,sound level");
#endif //ECHO_TO_SERIAL
 
 
 //COMMENTING THIS OUT RETURNS VALUES SO FAR.  IN WHOLE SKETCH NOT THE ANSWER
  // If you want to set the aref to something other than 5v 
//  analogReference(EXTERNAL);
}

 
 
void loop() 
{
  
    DateTime now;
      digitalWrite(greenLEDpin, HIGH);
      
       // fetch the time
  now = RTC.now();
// log time
//####### THIS LOGFILE BLOCK SEEMS TO INLFATE READINGS BY AROUND .1 #######
//  logfile.print(now.unixtime()); // seconds since 1/1/1970
//  logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
#if ECHO_TO_SERIAL
//  Serial.print(now.unixtime()); // seconds since 1/1/1970
//  Serial.print(", ");
  Serial.print(" Recording date:");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" Recording Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
#endif //ECHO_TO_SERIAL


    
   //fast sample (50 mS) 

   unsigned long startMillisF= millis();  // Start of sample window
   unsigned int peakToPeakF = 0;   // peak-to-peak level
 
   unsigned int signalMaxF = 0;
   unsigned int signalMinF = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillisF < sampleWindowF)
   {
      sampleF = analogRead(0);
      if (sampleF < 1024)  // toss out spurious readings
      {
         if (sampleF > signalMaxF)
         {
            signalMaxF = sampleF;  // save just the max levels
         }
         else if (sampleF < signalMinF)
         {
            signalMinF = sampleF;  // save just the min levels
         }
      }
   }
   peakToPeakF = signalMaxF - signalMinF;  // max - min = peak-peak amplitude
   double voltsF = (peakToPeakF * 3.3) / 1024;  // convert to volts
 
//   Serial.println(voltsF);
   
//   Serial.println(voltsS);
        logfile.print(", Volts Fast =  ");    
  logfile.print(voltsF);
#if ECHO_TO_SERIAL
  Serial.print(",  Volts Fast=  ");   
  Serial.print(voltsF);
#endif //ECHO_TO_SERIAL

     logfile.print(", Signal Max Fast =  ");    
  logfile.print(signalMaxF);
#if ECHO_TO_SERIAL
  Serial.print(",  Signal Max Fast =  ");   
  Serial.print(signalMaxF);
#endif //ECHO_TO_SERIAL

     logfile.print(", Signal Min Fast =  ");    
  logfile.print(signalMinF);
#if ECHO_TO_SERIAL
  Serial.print(",  Signal Min Fast =  ");   
  Serial.print(signalMinF);
#endif //ECHO_TO_SERIAL

//carriage return for data being logged
//  logfile.println();
//#if ECHO_TO_SERIAL
//  Serial.println();
//#endif // ECHO_TO_SERIAL
   delay(50);


//Slow sample (50 mS) 
 

   unsigned long startMillisS= millis();  // Start of sample window
   unsigned int peakToPeakS = 0;   // peak-to-peak level
 
   unsigned int signalMaxS = 0;
   unsigned int signalMinS = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillisS < sampleWindowS)
   {
      sampleS = analogRead(0);
      if (sampleS < 1024)  // toss out spurious readings
      {
         if (sampleS > signalMaxS)
         {
            signalMaxS = sampleS;  // save just the max levels
         }
         else if (sampleS < signalMinS)
         {
            signalMinS = sampleS;  // save just the min levels
         }
      }
   }
   peakToPeakS = signalMaxS - signalMinS;  // max - min = peak-peak amplitude
   double voltsS = (peakToPeakS * 3.3) / 1024;  // convert to volts
 
//   Serial.println(voltsS);
        logfile.print(", Volts Slow =  ");    
  logfile.print(voltsS);
#if ECHO_TO_SERIAL
  Serial.print(",  Volts Slow =  ");   
  Serial.print(voltsS);
#endif //ECHO_TO_SERIAL

     logfile.print(", Signal Max Slow =  ");    
  logfile.print(signalMaxS);
#if ECHO_TO_SERIAL
  Serial.print(",  Signal Max Slow =  ");   
  Serial.print(signalMaxS);
#endif //ECHO_TO_SERIAL

     logfile.print(", Signal Min Slow =  ");    
  logfile.print(signalMinS);
#if ECHO_TO_SERIAL
  Serial.print(",  Signal Min Slow =  ");   
  Serial.print(signalMinS);
#endif //ECHO_TO_SERIAL

//carriage return for data being logged
  logfile.println();
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL
   
   
   delay(50);
   
      //fast sample (50 mS) 

   unsigned long startMillisSPL= millis();  // Start of sample window
   unsigned int peakToPeakF = 0;   // peak-to-peak level
 
//   unsigned int signalMaxF = 0;
//   unsigned int signalMinF = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillisSPL < sampleWindowF)
   {
      sampleSPL = analogRead(1);
//      if (sampleF < 1024)  // toss out spurious readings
//      {
//         if (sampleF > signalMaxF)
//         {
//            signalMaxF = sampleF;  // save just the max levels
//         }
//         else if (sampleF < signalMinF)
//         {
//            signalMinF = sampleF;  // save just the min levels
//         }
//      }
   }
//   peakToPeakF = signalMaxF - signalMinF;  // max - min = peak-peak amplitude
//   double voltsF = (peakToPeakF * 3.3) / 1024;  // convert to volts
 
//   Serial.println(voltsF);
   
//   Serial.println(voltsS);
        logfile.print(", SPL  =  ");    
  logfile.print(sampleSPL);
#if ECHO_TO_SERIAL
  Serial.print(",  SPL =  ");   
  Serial.print(sampleSPL);
#endif //ECHO_TO_SERIAL


 digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

}
