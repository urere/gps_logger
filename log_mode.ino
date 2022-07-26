/**
 * Log Mode Functions
 */

/**
 * Setup Log mode
 */
void logMode_setup() {
  
  ledOn( RED_LED );

  // Initialise GPS
  initialiseGps();
  
  ledOff( RED_LED );  
}

/**
 * Log mode execution loop
 */
void logMode_loop() {
  
  // Read available byte
  gps.read();

  // Update information from GPS
  logMode_gpsUpdate();
  
  // Read available byte
  gps.read();

  // Update display
  logMode_displayUpdate( gps.fix, gps.speed );

  // Read available byte
  gps.read();

  // Update log
  logMode_logUpdate();
  
  // Read available byte
  gps.read();
}

/**
 * Obtained latest data from GPS
 */
void logMode_gpsUpdate() {

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
      sprintf( logFileName, "%2.2d%2.2d%2.2d.log", gps.day, gps.month, gps.year );
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

/**
 * Update display
 */
void logMode_displayUpdate( bool fix, nmea_float_t speed ) {

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
      display.println("LOG MODE");      
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

/**
 * Update log file
 */
void logMode_logUpdate() {
  
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
