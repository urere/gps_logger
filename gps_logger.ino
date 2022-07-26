#include <Adafruit_GPS.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>

// Hardware connections
#define GPS_SERIAL Serial1
#define DISPLAY_WIRE Wire
#define SD_CARD 4
#define RED_LED 13
#define GREEN_LED 8
#define MODE_BUTTON 5

// Configuration options
#define DISPLAY_INTERVAL_MS 2000
#define LOGGING_INTERVAL_MS 60000

// Mode
#define LOG_MODE 1
#define ADMIN_MODE 2

// Log mode display 
#define DISPLAY_SOG 1
#define DISPLAY_TIME_TO_LOG 2

// Common variables
int mode = LOG_MODE;
Adafruit_GPS gps(&GPS_SERIAL);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &DISPLAY_WIRE);
bool sdCardAvailable = false;

// Log mode variables
unsigned long displayTimer = millis();
unsigned long loggingTimer = millis();
bool gotFix = false;
char lastRMC[MAXLINELENGTH+1] = {0};
bool gotLogFileName = false;
char logFileName[13];
int displayMode = DISPLAY_SOG;

// Admin mode variables

/**
 * Setup
 */
void setup()
{
  // Attempt to initialize serial port
  Serial.begin(115200);
  delay(500);

  commonSetup();

  determineMode();
  
  if ( mode == LOG_MODE ) {
    logMode_setup(); 
  } else if ( mode == ADMIN_MODE ) {
    adminMode_setup(); 
  }
 
}

/**
 * Determine operation mode
 */
void determineMode() {

  for ( int i = 0; i < 20; i++ ) {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.print( "Hold for " );

    if ( (i % 2) == 0 ) {
      ledOn( RED_LED );
      display.println( ">" );
    } else {
      ledOff( RED_LED );
      display.println( " " );
    }

    display.print( "download" );
    display.display();
    
    delay(200);
  }
  
  ledOff( RED_LED );
  
  display.clearDisplay();
  display.display();

  if ( digitalRead(MODE_BUTTON) ) {
    mode = LOG_MODE;
  } else {
    mode = ADMIN_MODE;
  }
  
}

void ledOn( uint32_t led ) {

  digitalWrite( led, HIGH );
}

void ledOff( uint32_t led ) {

  digitalWrite( led, LOW );
}

void loop() {

  if ( mode == LOG_MODE ) {
    logMode_loop();
  } else if ( mode == ADMIN_MODE ) {
    adminMode_loop();
  }
}
