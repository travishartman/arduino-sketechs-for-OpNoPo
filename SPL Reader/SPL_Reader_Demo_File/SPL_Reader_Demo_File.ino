/* Test File For Reading in SPL Meter */
/*   Author: Ben Blier
 *   Copyright ©2014
 */


   /** Constants for Math **/
/* Microphone / Preamp Characteristics */
const int Sensitivity = -44;                         // dB re 1V/pA (See Data Sheet)
const float Transfer = 1000*pow(10, Sensitivity/20); // mV/pA
//const float Transfer = 6.3096;                     // mV/pA (Microphone Transfer, See data sheet)

const int Gain = 250;                          // Preamp Gain
const float dBGain = 32;//20*log10(Gain);          // In dB

/* uC Charactaristics */
const int DataSize = 32768/8;                 // Samples per Gathering. Too big = overlfow stdDev
const int DataMinus = DataSize - 1;
const int InputVoltage = 3300;                // mV
const int ADCrange = 1024;                    // sample range
const float mVscalar = InputVoltage/ADCrange; // mV / (adc sample) ** Division is messy do this up here!

const int average = ADCrange/2;   // Half of 1024. The signal is centered on "0" i.e. haflway

/** Variables **/
double stdDev = 0;                // Large floating point number
float dBSPL = 0;                  // normal sized floating point number
unsigned int i = 0;               // Coutning things!


 void setup ()
 {  
    
  Serial.begin(9600); 
 }
 
 
 
 void loop ()
 {
    // The code inside this loop will generate you one reading in about a half second
  
    stdDev = 0;
    dBSPL = 0;
    Serial.print("\n");
    Serial.println("Starting Read");
   
    // Now Calculate Standard Deviation (RMS value, without the offset)
    for (i = 0; i < DataSize; i++)
    {
      stdDev = stdDev + square(mVscalar * (analogRead(0) - average)); // Generate Sum part of standard deviation
    }	
    stdDev = sqrt(stdDev/DataMinus);  // Division Part of Standard Deviation
    
    // This value should be correct
    Serial.print("mV RMS: \t");
    Serial.println(stdDev);
    
    // Do the Conversion (This part doesn't seem to work):
    dBSPL = 20*log10(stdDev/Transfer) - dBGain + 94;
    Serial.print("dBSPL: \t\t");
    Serial.println(dBSPL);
    delay(250);
   
   
 }
