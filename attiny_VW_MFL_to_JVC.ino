//VW Multifunktionslenkrad auf JVC Radio Wandler
//Lenkrad-STG: 1J0 907 487 B
//Radio: JVC KD992BT 

//Thaddäus Köhler / Februar 2019

//Quelle des JVC Teils: https://hackaday.io/project/28150-jvc-to-clio/log/70875-jvc-part
//Verwendete Hardware:
//ATtiny85, 8 MHz interner Oszillator (WICHTIG! mit 1 MHz funktioniert die SoftwareSerial Bibliothek nicht)
//Sowohl das Lenkrad-STG als auch das JVC Radio arbeiten mit 5V-Pegeln. Daher wurden keine Transistoren oder Optokoppler verwendet.



#include <SoftwareSerial.h>
#define VOLUP       0x04
#define VOLDOWN     0x05
#define MUTE        0x0E
#define TRACKFORW   0x12
#define TRACKBACK   0x13
#define FOLDERFORW  0x14
#define FOLDERBACK  0x15 
 
//Ausgang zum Radio
#define JVC_REMOTE_PIN   4
 
// Pulseweite in µs
#define PULSEWIDTH 527
 
// Addresse des Radios (scheint immer gleich zu sein)
#define ADDRESS 0x47


// Initialize Serial Buffer
int incomingByte[13];

//Es muss nur empfangen werden
SoftwareSerial mySerial(0, 99); // RX, TX

void setup()
{

  //Baudrate für Software-Serial
  //Messung mit Oszi hat gezeigt, das 32 Bit Daten in 40 ms übertragen werden.
  //Das entspricht 800 baud. SoftwareSerial kennt 600, 1200 und 2400 baud. 2400 ist 800 x 3 (Überraschung :D)
  mySerial.begin(2400);

  // JVC Pin als Ausgang festlegen
  pinMode(JVC_REMOTE_PIN, OUTPUT); 
  //Pin auf Masse ziehen. Active High Signal
  digitalWrite(JVC_REMOTE_PIN, LOW);  
}

void loop() 
{

  for (int i = 0; i < 13; i++) {
    //Warte auf Daten vom Lenkrad
    while (!mySerial.available()); 
    //Speichere Daten in Array
    incomingByte[i] = mySerial.read();
    
    /*Sonderfall Taste gedrückt halten:
     Wird am Lenkrad eine Taste gedrückt gehalten, wird ein Repeat-Signal übertragen 
     Aussehen ungefähr so:  9 ms LOW, 2.5 ms HIGH, 0.6 ms LOW
     Software Serial ließt daraus ein 0xFF
     
     Die anderen Signale enden auch mit 0xFF.
     
     Wird ein 0xFF erkannt, wird die Schleife verlassen.
     Wenn es sich um ein Repeat-Signal handelt, wird die Schleife sofort verlassen,
     im Array stehen also noch die Werte vom vorherigen Tastendruck
    */
    if (incomingByte[i] == 0xFF)
      i = 13;
  }


  /*Es werden pro Tastendruck 4 Bytes übertragen:
  1. 0x41 (immer)
  2. 0xE8 (immer)
  3. Taste (z.B. 0x50 für "Runter")
  4. Checksumme (0xFF - 3. Byte)
  
  Das kann man per Oszi wunderbar sehen/ermitteln
  
  SoftwareSerial stellt die Daten anders da, was aber nicht weiter schlimm ist
  Im folgenden werden die Tasten unterschieden. Die ersten Bits werden ignoriert,
  da sie immer gleich sein. Die Checksumme wird auch nicht geprüft, da es so schon
  funktoniert. 
  */
  
  //Volume +
  if (incomingByte[6] == 0xCB && incomingByte[7] == 0x4B)
  {
    
    SendCommand(VOLUP);
  }
  //Volume -
  else if (incomingByte[6] == 0x4B && incomingByte[7] == 0x4B)
  {
    SendCommand(VOLDOWN);
  }
  //Taste Hoch (weiter)
  else if (incomingByte[6] == 0xCB && incomingByte[7] == 0x4F)
  {
   SendCommand(TRACKFORW);
  }
  //Taste Runter (zurück)
  else if (incomingByte[6] == 0x4B && incomingByte[7] == 0x7B)
  {
    SendCommand(TRACKBACK);
  }
}



//Bitbanging:


// Send a command to the radio, including the header, start bit, address and stop bits
void SendCommand(unsigned char value) {
  unsigned char i;
  Preamble();                         // Send signals to precede a command to the radio
  for (i = 0; i < 3; i++) {           // Repeat address, command and stop bits three times so radio will pick them up properly
    SendValue(ADDRESS);               // Send the address
    SendValue((unsigned char)value);  // Send the command
    Postamble();                      // Send signals to follow a command to the radio
  }
}

// Send a value (7 bits, LSB is sent first, value can be an address or command)
void SendValue(unsigned char value) {
  unsigned char i, tmp = 1;
  for (i = 0; i < sizeof(value) * 8 - 1; i++) {
    if (value & tmp)  // Do a bitwise AND on the value and tmp
      SendOne();
    else
      SendZero();
    tmp = tmp << 1; // Bitshift left by 1
  }
}

 
// Signals to transmit a '0' bit
void SendZero() {
//      Serial.println("zero");
  digitalWrite(JVC_REMOTE_PIN, HIGH);      // Output HIGH for 1 pulse width
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(JVC_REMOTE_PIN, LOW);       // Output LOW for 1 pulse width
  delayMicroseconds(PULSEWIDTH);
}
 
// Signals to transmit a '1' bit
void SendOne() {
//        Serial.println("uno");
  digitalWrite(JVC_REMOTE_PIN, HIGH);      // Output HIGH for 1 pulse width
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(JVC_REMOTE_PIN, LOW);       // Output LOW for 3 pulse widths
  delayMicroseconds(PULSEWIDTH * 3);
}
 
// Signals to precede a command to the radio
void Preamble() {
  // HEADER: always LOW (1 pulse width), HIGH (16 pulse widths), LOW (8 pulse widths)
  digitalWrite(JVC_REMOTE_PIN, LOW);       // Make sure output is LOW for 1 pulse width, so the header starts with a rising edge
  delayMicroseconds(PULSEWIDTH * 1);
  digitalWrite(JVC_REMOTE_PIN, HIGH);      // Start of header, output HIGH for 16 pulse widths
  delayMicroseconds(PULSEWIDTH * 16);
  digitalWrite(JVC_REMOTE_PIN, LOW);       // Second part of header, output LOW 8 pulse widths
  delayMicroseconds(PULSEWIDTH * 8);
 
  // START BIT: always 1
  SendOne();
}
 
// Signals to follow a command to the radio
void Postamble() {
  // STOP BITS: always 1
  SendOne();
  SendOne();
}

