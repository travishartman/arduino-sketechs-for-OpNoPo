/* Read in SPL. */
/*   Author: Ben Blier
 *   Copyright ©2014
 */
 
 
/*****************************************/
/**** THIS GOES AT TOP OF MAIN SKETCH ****/
/*****************************************/
* Constants for Math **/
/* Microphone / Preamp Characteristics */
/* The Mic_Sensitivity must be calibrated. */
/* The ADMP401 ranges from -45 to -39. Typical value is -42 */
const int Mic_Sensitivity = -44;                         // dB re 1V/pA (See Data Sheet)
const float Mic_Transfer = 1000*pow(10, Mic_Sensitivity/20); // mV/pA


const int Mic_Gain = 67;                             // Preamp Gain for <https://www.sparkfun.com/products/9868>
const float dBMic_Gain = 20*log10(Mic_Gain);         // In dB

/* uC Charactaristics */
const int Mic_NumSamples = 32768/8;                  // Samples per Gathering. Too big = overlfow stdDev
const int Mic_NumSamplesMinus = Mic_NumSamples - 1;  // Need this number for math. Why do the subtraction in the function?
const int Mic_InputVoltage = 3300;                   // mV
const int Mic_ADCrange = 1024;                       // sample range
const float Mic_mVscalar = Mic_InputVoltage/Mic_ADCrange; // mV / (adc sample) ** Division is messy, do this up here!

const int Mic_average = Mic_ADCrange/2;   // Half of ADC Range. The signal is centered on "0" i.e. haflway
/****************************************/
/****************************************/


/****************************************/
/** Don't include the setup and loop
/** from here, they're example usage
/****************************************/
void setup ()
{  
    
  Serial.begin(9600); 
}
 
void loop()
{
    Serial.print("\n");
    Serial.println("Starting Read");
   

   
   Serial.print("dBSPL: \t\t");
    Serial.println(GatherWorldNoise());
    delay(250);
}
 
/****************************************/
/** THIS GOES AT BOTTOM OF MAIN SKETCH **/
/****************************************/
 
float GatherWorldNoise(void)
{
    /** Variables **/
    double stdDev = 0;                // Large floating point number
    float dBSPL = 0;                  // normal sized floating point number
    unsigned int i = 0;               // counting things! 
    
    // Now Calculate Standard Deviation (RMS value, without the offset)
    for (i = 0; i < Mic_NumSamples; i++)
    {
      stdDev = stdDev + square(Mic_mVscalar * (analogRead(0) - Mic_average)); // Generate Sum part of standard deviation
    }	
    stdDev = sqrt(stdDev/Mic_NumSamplesMinus);  // Division Part of Standard Deviation
    
    
    // Do the Conversion to dBSPL:
    dBSPL = 20*log10(stdDev/Mic_Transfer) - dBMic_Gain + 94;
    
    return(dBSPL);
}
