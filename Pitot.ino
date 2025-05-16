// include the library code:
#include <LiquidCrystal.h>
#include <Wire.h>
#include <math.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Sensirion address
const char sensorAddress=0x25;

// Constants for sensirion data calculation
const int pressureFactor=60;
const int tempFactor=200;

// Constants for airspeed calculation
const double a0=331.3;
const double cas=0.606;
const double p0=101325;

// Variable for current airspeed

// State machine state variable
int screen=0;

// Battery voltage variable
float batteryVoltage=0;

// Airspeed average
double airSpeed=0;

//Alpha for averaging, 0.05 is a good starting value
float alpha = 0.05;

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.clear();
  lcd.print("Startup!");
  Wire.begin();        // join i2c bus (address optional for master)
  Wire.beginTransmission(sensorAddress); // transmit to device
  Wire.write(0x36);        // sends two bytes
  Wire.write(0x15);              // sends one byte
  Wire.endTransmission();    // stop transmitting
  delay(20);  
}

void loop() {
  int rawp, rawt; //raw values for pressure and temp
  float p, v; //computed actual values for pressure and temp

  //read the buttons and switch screen
  switch (read_LCD_buttons()) 
 {
   case btnUP:
     {
     if (screen<1) {
      screen++;
     }
     break;
     }
   case btnDOWN:
     {
     if (screen>0) {
      screen--;
     }
     break;
     }
 }

  //Read the pressure sensor and compute values
  Wire.requestFrom(sensorAddress, 5);    // request 3 bytes from slave device #8
  while (Wire.available()>0)  {
    rawp= Wire.read()<<8; //read raw pressure
    rawp|= Wire.read();
    Wire.read();
    rawt= Wire.read()<<8; //read raw temp
    rawt|= Wire.read();
  }
  p= (float)rawp/pressureFactor; //calculate delta p
  v= (a0+((rawt/tempFactor)*cas))*sqrt(5*(pow((fabs(p)/p0)+1,0.2857)-1)); // calculate windspeed in m/s
  airSpeed=(alpha*v)+((1.0-alpha)*airSpeed);
  //Read battery voltage
  batteryVoltage=(float)analogRead(3)*5/1025;
  
  //Print first line with computed pressure and temp
  lcd.home();
  if (rawp>=0) {
    lcd.print(" ");
  }
  lcd.print(p);
  lcd.print("Pa,");
  
  if (rawt>=0) {
    lcd.print(" ");
  }
  lcd.print((float)rawt/tempFactor);
  lcd.print("C ");

  //Print second line according to display setting
  switch (screen) {
    case 0: { 
      lcd.setCursor(0,1);
      lcd.print("V=");
      lcd.print(airSpeed);
      lcd.print("m/s,");
      lcd.print(v*3.6);
      lcd.print("kmh");
      break;
    }
    case 1: {
      lcd.setCursor(0,1);
      lcd.print("Vb=");
      lcd.print(batteryVoltage);
      lcd.print("V             ");
      break;
    }
  }
  
  //Wait and repeat
  delay(200);
}

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor 
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return btnRIGHT;  
  if (adc_key_in < 250)  return btnUP; 
  if (adc_key_in < 450)  return btnDOWN; 
  if (adc_key_in < 650)  return btnLEFT; 
  if (adc_key_in < 850)  return btnSELECT;  

  return btnNONE;  // when all others fail, return this...
}


