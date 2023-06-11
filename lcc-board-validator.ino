#include <ACAN2515.h>

static const byte MCP2515_CS  = 10 ; // CS input of MCP2515 (adapt to your design) 
static const byte MCP2515_INT =  2 ; // INT output of MCP2515 (adapt to your design)
static const byte EEPROM_CS = 9;
static const byte LED_1 = 5;
static const byte LED_2 = 6;

ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;

static const uint32_t QUARTZ_FREQUENCY = 16UL * 1000UL * 1000UL ; // 16 MHz

int found_can = 0;
int found_eeprom = 0;

void find_eeprom(){
  digitalWrite(EEPROM_CS, LOW);
  SPI.transfer(0x06); // Write enable
  digitalWrite(EEPROM_CS, HIGH);

  delay(5);

  digitalWrite(EEPROM_CS, LOW);
  SPI.transfer(0x05); // read stataus register
  int read_status_reg = SPI.transfer(0xFF);
  digitalWrite(EEPROM_CS, HIGH);

  delay(5);

  if(read_status_reg & (0x01 << 1)){
    // WEL bit is set, so we are talking with the EEPROM!
    // Let's go and disable it again
    found_eeprom = 1;
    digitalWrite(EEPROM_CS, LOW);
    SPI.transfer(0x04); // Write disable
    digitalWrite(EEPROM_CS, HIGH);
    Serial.println("Found EEPROM!");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin (9600) ;

  SPI.begin () ;
  Serial.println ("Configure ACAN2515") ;
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = 4;
  settings.mTransmitBuffer0Size = 8;
  const uint16_t errorCode = can.begin (settings, [] { can.isr () ; }) ;
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }else{
    found_can = 1;
    Serial.println("Found MCP2515");
  }

  // LEDs
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  
  // EEPROM CS
  pinMode(EEPROM_CS, OUTPUT);

  // Try to talk with the EEPROM
  find_eeprom();
}

void blinkLEDs(){
  digitalWrite(LED_1, 1);
  digitalWrite(LED_2, 1);
  delay(200);
  digitalWrite(LED_1, 0);
  digitalWrite(LED_2, 0);
}

void loop() {
  // Blink sequence:
  // All good = One blink every three seconds
  // No MCP2515 = two blinks every three seconds
  // No EEPROM = three blinks every three seconds
  // No MCP2515 or EEPROM = four blinks every three seconds
  if(millis() % 3000 == 0){
    if(found_can && found_eeprom){
      blinkLEDs();
    }else if(!found_can && found_eeprom){
      blinkLEDs();
      delay(200);
      blinkLEDs();
    }else if(found_can && !found_eeprom){
      blinkLEDs();
      delay(200);
      blinkLEDs();
      delay(200);
      blinkLEDs();
    }else if(!found_can && !found_eeprom){
      blinkLEDs();
      delay(200);
      blinkLEDs();
      delay(200);
      blinkLEDs();
      delay(200);
      blinkLEDs();
    }
  }
}
