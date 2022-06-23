#include <Adafruit_GPS.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>

// Configuration options
#define DISPLAY_INTERVAL_MS 2000
#define LOGGING_INTERVAL_MS 60000

// Hardware connections
#define GPS_SERIAL Serial1
#define DISPLAY_WIRE Wire
#define SD_CARD 4
#define DEBUG Serial
#define RED_LED 13
#define GREEN_LED 8

// Global variables
Adafruit_GPS gps(&GPS_SERIAL);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &DISPLAY_WIRE);
uint32_t displayTimer = millis();
uint32_t loggingTimer = millis();
bool sdCardAvailable = false;
char *lastRMC = NULL;
bool gotLogFileName = false;
char logFileName[13];

void setup()
{
  // Initialise DEBUG
  // while (!DEBUG);  // uncomment to have the sketch wait until Serial is ready
  DEBUG.begin(115200);

  // LEDs
  initialiseLEDs();

  ledOn( RED_LED );

  // Initialise Display
  initialiseDisplay();

  // Initialise SD card
  initialiseSDCard();

  // Initialise GPS
  initialiseGps();
  
  ledOff( RED_LED );
}

void initialiseLEDs() {
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

}

void initialiseDisplay() {
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
}

void initialiseGps() {
  
  gps.begin(9600);
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  gps.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPS_SERIAL.println(PMTK_Q_RELEASE);
}

void initialiseSDCard() {
  sdCardAvailable = SD.begin(SD_CARD);
}

void ledOn( uint32_t led ) {

  digitalWrite( led, HIGH );
}

void ledOff( uint32_t led ) {

  digitalWrite( led, LOW );
}

void displayUpdate( bool fix, nmea_float_t speed ) {

  // Check for screen update
  if (millis() - displayTimer > DISPLAY_INTERVAL_MS) {
    displayTimer = millis(); 

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);

    if (fix) {
      display.print( "SOG: " );
      display.println(speed );
      if ( gotLogFileName ) {
        display.print(logFileName);
      }
    } else {
      display.println("NO FIX");      
      if ( gotLogFileName ) {
        display.print(logFileName);
      }
    }
    display.display();
    
  }
  
}

void logUpdate() {
  
  // Check for screen update
  if (millis() - loggingTimer > LOGGING_INTERVAL_MS) {
    loggingTimer = millis();

    if ( sdCardAvailable ) {
      if ( lastRMC != NULL ) {
      
       ledOn( GREEN_LED );

      // Actually write the log file entry
       File logFile = SD.open(logFileName, FILE_WRITE);
       if ( logFile ) {
         logFile.print( lastRMC );
         logFile.close();
       }
       lastRMC = NULL;
    
       ledOff( GREEN_LED );
      }
    }
  
  }
  
}

void loop() // run over and over again
{
  gps.read();
  if (gps.newNMEAreceived()) {
    
    if (!gps.parse(gps.lastNMEA())) { 
      ledOn( RED_LED );
      return; 
    }
    ledOff( RED_LED );
    
    if ( gps.fix ) {

      // Update log file name
      sprintf( logFileName, "%2.2d%2.2d%2.2d.log", gps.year, gps.month, gps.day );
      gotLogFileName = true;

      // Need to store the last RMS sentance so it can be logged
      if ( strcmp( gps.lastSentence, "RMC" ) == 0 ) {
        lastRMC = gps.lastNMEA();
      }  
    }      
  }

  displayUpdate( gps.fix, gps.speed );

  logUpdate();
  
}
