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

// Display constants
#define DISPLAY_SOG 1
#define DISPLAY_TIME_TO_LOG 2

// Global variables
Adafruit_GPS gps(&GPS_SERIAL);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &DISPLAY_WIRE);
unsigned long displayTimer = millis();
unsigned long loggingTimer = millis();
bool sdCardAvailable = false;
bool gotFix = false;
char lastRMC[MAXLINELENGTH+1] = {0};
bool gotLogFileName = false;
char logFileName[13];
int displayMode = DISPLAY_SOG;

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

void gpsUpdate() {

  char *pLastNMEA;
  
  // Check for new sentance
  if (gps.newNMEAreceived()) {

    // Get last sentance and attempt to parse it
    pLastNMEA = gps.lastNMEA();
    if (!gps.parse(pLastNMEA)) { 
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
        // Remove checksum (*nn) and cr/lf
        String lastNMEA = String(pLastNMEA);
        int ck = lastNMEA.lastIndexOf('*');
        if ( ck > 0 ) {
          lastNMEA.remove(ck);
          lastNMEA.toCharArray(lastRMC, MAXLINELENGTH+1);
        }
      }

      if ( !gotFix ) {
        // Just got a fix so restart the logging timer
        gotFix = true;
        loggingTimer = millis();
      }
      
    } else {
      
      if ( gotFix ) {
        // Just lost the fix
        gotFix = false;
      }
    }
  }
  
}

void displayUpdate( bool fix, nmea_float_t speed ) {

  // Check for screen update
  unsigned long m = millis();
  if (m - displayTimer > DISPLAY_INTERVAL_MS) {
    displayTimer = millis(); 

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);

    if (fix) {

      if ( displayMode == DISPLAY_SOG ) {
        
        display.print( "SOG: " );
        display.println(speed );
        displayMode = DISPLAY_TIME_TO_LOG;
        
      } else if ( displayMode == DISPLAY_TIME_TO_LOG ) {

        // Display a simple progress bar
        display.println();
        display.drawRect( 0, 0, 120, 7, SSD1306_WHITE );
        int p = map( m - loggingTimer, 0, LOGGING_INTERVAL_MS, 0, 120 );
        display.fillRect( 0, 1, p, 5, SSD1306_WHITE );
         
        displayMode = DISPLAY_SOG;
        
      } else {
        
        display.println();
      }
      
    } else {
      display.println("NO FIX");      
      displayMode = DISPLAY_SOG;
    }

    // Last line is always the log file if there is a fix
    if ( gotLogFileName && fix ) {
      if ( sdCardAvailable ) {
          display.print(logFileName);
      } else {
          display.print("NO CARD");
      }
    }

    // Update the display
    display.display();
    
  }
  
}

void logUpdate() {
  
  // Check for screen update
  if (millis() - loggingTimer > LOGGING_INTERVAL_MS) {
    loggingTimer = millis();

    if ( sdCardAvailable ) {
      if ( lastRMC[0] != 0 ) {
      
       ledOn( GREEN_LED );

      // Actually write the log file entry
       File logFile = SD.open(logFileName, FILE_WRITE);
       if ( logFile ) {
         logFile.println( lastRMC );
         logFile.close();
       }
       lastRMC[0] = 0;
    
       ledOff( GREEN_LED );
      }
    }
  
  }
  
}

void loop() // run over and over again
{
  // Read available byte
  gps.read();

  // Update information from GPS
  gpsUpdate();
  
  // Read available byte
  gps.read();

  // Update display
  displayUpdate( gps.fix, gps.speed );

  // Read available byte
  gps.read();

  // Update log
  logUpdate();
  
  // Read available byte
  gps.read();
}
