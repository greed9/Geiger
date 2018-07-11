//DFRobot.com
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
const byte neopixelPin = 9 ;

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

#define CPM_CAL_FACTOR 0.0021
#define BED_CAL_FACTOR 0.1

const byte isrPin = 2 ;
const byte clickerPin = 12 ;
volatile int clickCount = 0 ;
volatile int state = 0 ;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, neopixelPin, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

LiquidCrystal_I2C lcd(0x27, 4, 20); // set the LCD address to 0x3f for a 16 chars and 2 line display

void setAllPixels ( Adafruit_NeoPixel& pix, int red, int green, int blue )
{
  for ( int i = 0 ; i < NUMPIXELS ; i++ )
  {
    pix.setPixelColor( i, pix.Color (red, green, blue )) ;
    delay( 1 ) ;
  }
  pix.show ( ) ;
}

void countPulses ( )
{
  clickCount ++ ;
}

void setup()
{
  lcd.init();                      // initialize the lcd

  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear ( ) ;
  printStr( "Desktop Geiger") ;
  delay( 5000 ) ;
  lcd.clear ( ) ;

  pixels.begin(); // This initializes the NeoPixel library
  setAllPixels ( pixels, 8, 0, 4 ) ;

  pinMode( clickerPin, OUTPUT ) ;
  pinMode( isrPin, INPUT_PULLUP ) ;
  attachInterrupt(digitalPinToInterrupt(isrPin), countPulses, FALLING);

  // setup for serial plotter

  Serial.begin( 9600 ) ;
  /*
    Serial.println ( 100 ) ;
    Serial.println( 100 ) ;
    delay( 5000 ) ;
  */
}

void printStr ( char* str )
{
  for ( int i = 0 ; i < strlen ( str ) ; i++ )
  {
    lcd.print( str[i] ) ;
  }
}

int totalThisMinute = 0 ;

void updatePerMinute ( int diff )
{
  static int tenSecAvg = 0 ;
  static unsigned long startTime = millis ( ) ;
  static int tenSecondCount = 1 ;
  unsigned long timeNow = millis ( ) ;
  char temp[30] = { 0 } ;

  totalThisMinute += diff ;

  if ( tenSecondCount == 6 )
  {
    lcd.clear ( ) ;
    tenSecondCount = 1 ;
    lcd.setCursor( 0, 0 ) ;
    sprintf ( temp, "%d", totalThisMinute ) ;
    printStr ( temp ) ;
    printStr ( " : CPM (actual)" ) ;

    // https://sites.google.com/site/diygeigercounter/gm-tubes-supported
    float microSv = ( float ) totalThisMinute * CPM_CAL_FACTOR ;

    // https://en.wikipedia.org/wiki/Banana_equivalent_dose
    float bed = microSv / BED_CAL_FACTOR ;
    lcd.setCursor( 0, 1 ) ;
    lcd.print( microSv, 2 ) ;
    printStr( " : uSv/hr" ) ;
    lcd.setCursor( 0, 2 ) ;
    lcd.print( bed, 2 ) ;
    printStr ( " : Bananas" ) ;
    totalThisMinute = 0 ;

  } else {

    if ( timeNow - startTime > 10000L )
    {
      tenSecAvg = totalThisMinute * ( float ) ( 6.0 / ( float ) tenSecondCount ) ;
      
      lcd.setCursor( 0, 0 ) ;
      sprintf ( temp, "%d", tenSecAvg ) ;
      printStr ( temp ) ;
      printStr (  " CPM (estimated)" ) ;
      startTime = millis ( ) ;
      tenSecondCount ++ ;
      Serial.print ( tenSecondCount ) ;
      Serial.print ( "," ) ;
      Serial.print ( totalThisMinute ) ;
      Serial.print( "," ) ;
      Serial.println( tenSecAvg ) ;

    }
  }

}

void loop()
{
  static int currentCount = 0 ;
  char temp[25] = { 0 } ;
  int diff = clickCount - currentCount ;

  if ( diff )
  {
    currentCount = clickCount ;

    updatePerMinute ( diff ) ;
    while ( diff )
    {
      digitalWrite( clickerPin, HIGH ) ;
      delay ( 10 ) ;
      digitalWrite( clickerPin, LOW ) ;

      setAllPixels ( pixels, 70, 0, 35 ) ;
      delay( 100 ) ;
      setAllPixels ( pixels, 8, 0, 4 ) ;
      diff-- ;
    }
  }
  delay( 10 ) ;
}
