/* Test File For Reading in SPL Meter */
/*   Author: Ben Blier
 *   Copyright ©2014
 */


   /** Constants for Math **/
const int DataSize = 32768;      // Samples per Gathering 
const float Transfer = 6.3096;  // mV/pA (Microphone Transfer, See data sheet)

/** Variables **/
//int OldData[DataSize];
long average = 512;
double stdDevLong = 0;
float dBSPL = 0;
unsigned int i = 0;


 void setup ()
 {  
    
  Serial.begin(9600); 
 }
 
 
 
 void loop ()
 {
    // The code inside this loop will generate you one reading in about a half second
  
    stdDevLong = 0;
    dBSPL = 0;
    Serial.print("\n");
    Serial.println("Starting Read");
    
    for (i = 0; i < DataSize; i++)
    {
      stdDevLong = stdDevLong + square(analogRead(0) - average); // Generate Sum
    }	
    //average = average / DataSize; // Division Part of Average


    // Now Calculate Standard Deviation (RMS value, without the offset)
//    for (i = 0; i < DataSize; i++)
//    {
//	stdDevLong = stdDevLong + square(OldData[i] - average); // Generate Sum
//    }
    stdDevLong = stdDevLong/(DataSize - 1);  // Standard Deviation
    
    Serial.print("StdDeviation: \t");
    Serial.println(stdDevLong);
    
    
    // I hope I set up this order of operations so that it doesn't overflow anything.
    // The Arduino is only 16-bit.
    stdDevLong = 3300/1024 * (stdDevLong/125); 	// Now we have the Standard Deviation in mV
    Serial.print("mV RMS: \t");
    Serial.println(stdDevLong);
    
    // Do the Conversion:
    dBSPL = (stdDevLong * 94) / Transfer; // This is the output!	
    Serial.print("dBSPL: \t\t");
    Serial.println(dBSPL);
    //delay(1000);
   
   
 }
