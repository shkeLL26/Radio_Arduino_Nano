#include <LiquidCrystal.h>

#include <LiquidCrystal_I2C_Menu.h>
#include <Wire.h>


#define RDA5807M_RANDOM_ACCESS_ADDRESS 0x11
uint8_t volume = 1;
uint16_t reg03h, reg05h, reg0Ah, reg0Ch, reg0Dh, reg0Eh, reg0Fh, test, band, space, r_chan, button, stc, rds_ready;
char letter;
char lfalse = 0;
LiquidCrystal_I2C_Menu lcd(0x27, 16, 2);

int band_m[2] = {87, 76};
float space_m[2] = {0.1, 0.2};
float fm;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  setRegister(0x02, 0xC109); // set ENABLE, DHIZ, DMUTE, SEEK
  reg03h = getRegister(0x03);
  reg05h = getRegister(0x05);
  reg0Ah = getRegister(0x0A);
  stc = (reg0Ah & 0b0100000000000000) >> 14;
  band = (reg03h & 0b0000000000001100);
  space = (reg03h & 0b0000000000000011);
  letter = 0;
}

void loop() {
  reg0Ah = getRegister(0x0A);
  reg0Ch = getRegister(0x0C);
  reg0Dh = getRegister(0x0D);
  reg0Eh = getRegister(0x0E);
  reg0Fh = getRegister(0x0F);
  int n_struct = reg0Dh & 0x0003;
  rds_ready = (reg0Ah & 0x1000) >> 12; 
  //Serial.println(reg0Ch, HEX);
  //Serial.println(reg0Dh, HEX);
  //Serial.println(reg0Eh, HEX);
  //Serial.println(reg0Fh, BIN);
  //Serial.println(rds_ready, HEX);
  if (rds_ready == 1 and n_struct == 0x0 and lfalse == 0) {
    char letter = ((reg0Fh & 0xFF00) >> 8);
    char lfalse = letter;
    Serial.print(letter);
  }
  if (rds_ready == 1 and lfalse == letter) { 
     if (n_struct=0x1) {
      char letter = letter + char (reg0Fh & 0x00FF);
      Serial.print(letter);
      lfalse = letter;
     }
     else if (n_struct=0x2) {
      char letter = letter + char ((reg0Fh & 0xFF00) >> 8);
      Serial.print(letter);
      lfalse = letter;
     }
     else if (n_struct=0x3) {
      char letter = letter + char (reg0Fh & 0x00FF);
      Serial.println(letter);
      lfalse = letter;
     }
  } 
  button = analogRead(A0);
  if (button >= 590 and button <= 610) {
    setRegister(0x02, 0xC309);
    Serial.println("Station selected");
    stc=0;
    while(stc==0) {
     reg0Ah = getRegister(0x0A);
     delay(200);
     stc = (reg0Ah & 0b0100000000000000) >> 14;
    }
    Serial.println(stc);
    r_chan = (reg0Ah & 0b0000001111111111);
    fm = r_chan * space_m[space] + band_m[band];
    Serial.println(fm);
    lcd.clear();
    lcd.print(fm);   
  }
  test = reg05h & 0x000F;
  if (button >= 720 && button <= 740 && test < 0x000F) {
    Serial.println("Button_volume+");
    Serial.println(test, BIN);
    reg05h += 1;
    Serial.println(reg05h, BIN);
    setRegister(0x05, reg05h);
  }
  if (button >= 790 && button <= 815 && test > 0x0000) {
    Serial.println("Button_volume-");
    Serial.println(test, BIN);
    reg05h -= 1;
    Serial.println(reg05h, BIN);
    setRegister(0x05, reg05h);
  }
  delay(40);
  lcd.clear();
  lcd.print(fm);  
}

void setRegister(uint8_t reg, const uint16_t value) {
  Wire.beginTransmission(0x11);
  Wire.write(reg);
  Wire.write(highByte(value));
  Wire.write(lowByte(value));
  Wire.endTransmission(true);
}
uint16_t getRegister(uint8_t reg) {
  uint16_t result;
  Wire.beginTransmission(RDA5807M_RANDOM_ACCESS_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(0x11, 2, true);
  result = (uint16_t)Wire.read() << 8;
  result |= Wire.read();
  return result;
}
