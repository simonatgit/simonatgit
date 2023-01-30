/* Test HID card reader + RS485 with ESP-12F only (WORKS!!!!) 
 * Dated: 29/12/2022, 12:19am...
 * This copy is final and used at site
 * Modify by: Simon

PFFFFFFFFFFFFFFFFCCCCCCCCCCCCCCCCCCCP
EXXXXXXXXXXXXXXXXXX..................
..................XXXXXXXXXXXXXXXXXXO

P = Parity
O = Odd Parity
E = Even Parity
X = Parity mask
F = Facility Code, range = 0 to  65,535
C = Cardholder ID, range = 0 to 524,287
*/


#include "Arduino.h"
#include <SoftwareSerial.h>
#define RXPin        4  // Serial Receive pin (original 5)
#define TXPin        5  // Serial Transmit pin (original 4)
SoftwareSerial RS485Serial(RXPin, TXPin); // RX, TX
 
int byteSend;
 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse (3000) 
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code
String stringID, response;         // convert binary to string

int BLD_LED = 2;     // on board build in LED
//int LED_GREEN = 9; // orange wire, not working
//int LED_RED = 10;  // brown wire, not working
int BEEP = 14;  // yellow wire

// interrupt that happens when INTO goes low (0 bit)
ICACHE_RAM_ATTR void ISR_INT0() { // ICACHE_RAM_ATTR is needed before ISR function for ESP8266
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
// interrupt that happens when INT1 goes low (1 bit)
ICACHE_RAM_ATTR void ISR_INT1() {
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup() {
  pinMode(BLD_LED, OUTPUT);
//  pinMode(LED_RED, OUTPUT);  
//  pinMode(LED_GREEN, OUTPUT);  
  pinMode(BEEP, OUTPUT);  
//  digitalWrite(LED_RED, HIGH);   // High = Off
  digitalWrite(BEEP, HIGH);   // High = off
//  digitalWrite(LED_GREEN, LOW);  // Low = On
stringID = String("@");           // Starting symbol


Serial.begin(19200);
  
  digitalWrite(BLD_LED, HIGH);
  RS485Serial.begin(19200);        // set the data rate
  delay(500);                      // believe it take some time for setting data rate
  //Make sure the mode is INPUT_PULLUP for Software House SWH 5100 Model.
  pinMode(13, INPUT_PULLUP);       // DATA0 (INT0) 
  pinMode(12, INPUT_PULLUP);       // DATA1 (INT1)
  
//   binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(13, ISR_INT0, FALLING);  // for ESP, interrupt bit must follow pin#
  attachInterrupt(12, ISR_INT1, FALLING);  // for ESP, interrupt bit must follow pin#
 
 
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop()
{ 
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1; 
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
    for (i=0; i<bitCount; i++) 
     {
       stringID += databits[i];
     }
       stringID += ("#");
       
    Serial.print("\nBits = "); // not understand 
    Serial.println(bitCount);    // not understand

    
    // **************** our card is 37 bits ************
    
    if (bitCount == 37) {
      for (i=1; i<17; i++) {       // facility code = bits 2 to 17
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      for (i=17; i<36; i++) {       // card code = bits 18 to 36
         cardCode <<=1;
         cardCode |= databits[i];
      }
         printBits();                  // Sending data to Board 2
      }
    
    else{

     Serial.print("\nUnable to decode.");
     Serial.print("\n\n**************************\n\n");
     RS485Serial.print(stringID);

        }
    
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     stringID = String("@");
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
}


     if (RS485Serial.available()){                   // check if there is any Serial data received
         Serial.println("Response available!");
         response = RS485Serial.readString();
         Serial.println(response);   // print the data Board 2 received as confirmation
         digitalWrite(BLD_LED, HIGH);                // turns OFF LED

         if (response == "NON37"){
            digitalWrite(BEEP, LOW);
            delay(1500);
            digitalWrite(BEEP, HIGH);}
         else if (response == "MAINT"){
            digitalWrite(BEEP, LOW);
            delay(100);
            digitalWrite(BEEP, HIGH);
            delay(100);
            digitalWrite(BEEP, LOW);
            delay(100);
            digitalWrite(BEEP, HIGH);
            delay(100);
            digitalWrite(BEEP, LOW);
            delay(1500);
            digitalWrite(BEEP, HIGH);
            delay(500);
            digitalWrite(BEEP, LOW);
            delay(1500);
            digitalWrite(BEEP, HIGH);
         response = "";
         }
         }
}



void printBits(){

Serial.println(stringID);    // for serial monitoring purpose only
RS485Serial.print(stringID  ); // Send message 
Serial.println("Send data!");

      digitalWrite(BLD_LED, LOW); // turns ON LED
      Serial.print("\nFacility Code = ");
      Serial.print(facilityCode);
      Serial.print("\nCard ID = ");
      Serial.print(cardCode);
      Serial.print("\n\n**************************\n\n");

      // Quick beeps for 37 bits card
      digitalWrite(BEEP, LOW);
      delay(100);
      digitalWrite(BEEP, HIGH);
      delay(30);
      digitalWrite(BEEP, LOW);
      delay(100);
      digitalWrite(BEEP, HIGH);
      delay(30);
}
