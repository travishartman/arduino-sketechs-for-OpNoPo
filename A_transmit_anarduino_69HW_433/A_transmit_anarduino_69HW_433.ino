//MUST SET BOARD TO ANARDUINO WIRELESS BOARD TO DEMILANOVE ATMEGA328 FOR IT TO WORK
//combined with sound sketch- verifies- need to integrate the two with varibales to send sound info now-
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
//#include <JeeLib.h>
//ISR(WDT_vect) { Sleepy::watchdogEvent(); }
#include <LowPower.h>



#define NODEID      99
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF69_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack



const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;



int TRANSMITPERIOD = 3000; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;

typedef struct {		
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  //  float         temp;   //temperature maybe?
  double         volts;   //temperature maybe?
} 
Payload;
Payload theData;

void setup() {
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);

  //  if (flash.initialize())
  //    Serial.println("SPI Flash Init OK!");
  //  else
  //    Serial.println("SPI Flash Init FAIL! (is chip present?)");
}

long lastPeriod = -1;
//LOOP HEERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void loop() {
  
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
    }


    if (input == 'r') //d=dump register values
      radio.readAllRegs();
    //if (input == 'E') //E=enable encryption
    //  radio.encrypt(KEY);
    //if (input == 'e') //e=disable encryption
    //  radio.encrypt(null);

    if (input == 'd') //d=dump flash area
    {
      Serial.println("Flash content:");
      int counter = 0;

      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        Serial.print('.');
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'e')
    {
      Serial.print("Erasing Flash chip ... ");
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
  }




  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');
    Serial.print(radio.SENDERID, DEC);
    Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");
    Serial.print(radio.readRSSI());
    Serial.print("]");

    if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      delay(10);
    }
    Blink(LED,5);
    Serial.println();
  }

  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    
//for (byte i = 0; i < 74; ++i)
//  {
//    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
//  }
    
    unsigned long startMillis= millis();  // Start of sample window
    unsigned int peakToPeak = 0;   // peak-to-peak level

    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow)
    {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
        if (sample > signalMax)
        {
          signalMax = sample;  // save just the max levels
        }
        else if (sample < signalMin)
        {
          signalMin = sample;  // save just the min levels
        }
      }
    }
    peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

    //fill in the struct with new values
    theData.nodeId = NODEID;
    theData.uptime = millis();
    //    theData.temp = 91.23; //it's hot!
    theData.volts = volts; //it's hot!

    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData)))
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println();
    Blink(LED,3);
    lastPeriod=currPeriod;
    //    for (byte i = 0; i < 120; ++i)
    //      Sleepy::loseSomeTime(3000);
  }
}



void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);

}


