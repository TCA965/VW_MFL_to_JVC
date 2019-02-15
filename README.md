# VW_MFL_to_JVC

VW Multifunktionslenkrad auf JVC Radio Wandler
Lenkrad-STG: 1J0 907 487 B
Radio: JVC KD992BT 

Thaddäus Köhler / Februar 2019

Quelle des JVC Teils: https://hackaday.io/project/28150-jvc-to-clio/log/70875-jvc-part
Verwendete Hardware:
ATtiny85, 8 MHz interner Oszillator (WICHTIG! mit 1 MHz funktioniert die SoftwareSerial Bibliothek nicht)
Sowohl das Lenkrad-STG als auch das JVC Radio arbeiten mit 5V-Pegeln. Daher wurden keine Transistoren oder Optokoppler verwendet.
