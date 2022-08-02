/**
   Admin Mode Functions
*/

/**
   Setup Admin mode
*/
void adminMode_setup() {

  // Leave red LED on to indicate admin mode
  ledOn( RED_LED );

  if ( sdCardAvailable ) {
    adminMode_displayWaitingMsg();
  } else {
    adminMode_displayMsg( "NO CARD" );
  }

  // Setup serial commands
  serialCommands.addError(&adminMode_ErrorHandler);
  serialCommands.addExecuteCommand((char *)"HELP", &adminMode_help);
  serialCommands.addExecuteCommand((char *)"LIST", &adminMode_list);
  serialCommands.addExecuteCommand((char *)"GET", &adminMode_get);
  serialCommands.addExecuteCommand((char *)"GETGPX", &adminMode_getGPX);
  serialCommands.addExecuteCommand((char *)"DELETE", &adminMode_delete);
  serialCommands.addExecuteCommand((char *)"DELETEALL", &adminMode_deleteAll);

  serialCommands.begin( Serial );
}

/**
   Execution loop for admin mode
*/
void adminMode_loop() {

  serialCommands.loop();
}

/**
   List available commands
*/
void adminMode_help() {

  adminMode_displayMsg( "HELP" );

  adminMode_sendRespStart();
  adminMode_sendRespLine( "HELP              (Display available commands)" );
  adminMode_sendRespLine( "LIST              (List available logs)" );
  adminMode_sendRespLine( "GET=<logname>     (Download log in NMEA format)" );
  adminMode_sendRespLine( "GETGPX=<logname>  (Download log in GPX format)" );
  adminMode_sendRespLine( "DELETE=<logname>  (Delete log)" );
  adminMode_sendRespLine( "DELETEALL         (Delete all logs)" );
  adminMode_sendRespEnd();

  adminMode_displayWaitingMsg();
}

/**
   List available logs
*/
void adminMode_list() {


  if ( adminMode_checkCardAvailable() ) {
  
    adminMode_displayMsg( "LIST" );

    adminMode_sendRespStart();

    // Just scan the root directory for log files
    File root = SD.open("/");
    File entry =  root.openNextFile();
    while ( entry != NULL ) {
      if ( !entry.isDirectory() ) {
        // Remove any file extension
        String logName = String(entry.name());
        int ext = logName.lastIndexOf('.');
        if ( ext > 0 ) {
          logName.remove(ext);
        }        
        adminMode_sendRespLine( (char *) logName.c_str() );
      }
      entry =  root.openNextFile();
    }

    adminMode_sendRespEnd();

    adminMode_displayWaitingMsg();
  }

}

/**
   Get log in raw NMEA format
*/
void adminMode_get() {


  if ( adminMode_checkCardAvailable() ) {

    adminMode_displayMsg( "GET" );

    char *pArg;

    // Read argument that contains the log name
    pArg = serialCommands.next();

    if ( pArg != NULL ) {

      // Add file extension
      String logFileName = String(pArg);
      logFileName.concat(LOG_FILE_EXT); 

      // Attempt to open the log file
      File logFile = SD.open((char *) logFileName.c_str());
      if ( logFile != NULL ) {

        adminMode_sendRespStart();
        
        // Stream the file back
        while (logFile.available()) {
          Serial.write(logFile.read());
        }
        logFile.close();
        
        adminMode_sendRespEnd();
        
      } else {
        adminMode_sendRespError( "UNABLE TO OPEN LOG FILE" );
      }
    } else {
      adminMode_sendRespError( "GET=<LOG>: LOG MISSING" );
    }

    adminMode_displayWaitingMsg();
  }

}

/**
   Get log in GPX format
*/
void adminMode_getGPX() {

  if ( adminMode_checkCardAvailable() ) {

    adminMode_displayMsg( "GETGPX" );

    char *pArg;

    // Read argument that contains the log name
    pArg = serialCommands.next();

    if ( pArg != NULL ) {
      adminMode_sendRespStart();
      adminMode_sendRespLine( "GETGPX command executed" );
      adminMode_sendResp( "  LOG:" );
      adminMode_sendRespLine( pArg );
      adminMode_sendRespEnd();
    } else {
      adminMode_sendRespError( "GETGPX=<LOG>: LOG MISSING" );
    }

    adminMode_displayWaitingMsg();
  }

}

/**
   Delete a log
*/
void adminMode_delete() {

  if ( adminMode_checkCardAvailable() ) {

    adminMode_displayMsg( "DELETE" );

    char *pArg;

    // Read argument that contains the log name
    pArg = serialCommands.next();

    if ( pArg != NULL ) {

      // Add file extension
      String logFileName = String(pArg);
      logFileName.concat(LOG_FILE_EXT);
      
      // Attempt to delete the file
      if ( SD.remove(logFileName) == 1 ) {
        adminMode_sendRespStart();
        adminMode_sendRespLine( "DELETE=<LOG>: DELETED" );
        adminMode_sendRespEnd();
      } else {
        adminMode_sendRespError( "DELETE=<LOG>: DELETE FAILED" );
      }
      
    } else {
      adminMode_sendRespError( "DELETE=<LOG>: LOG MISSING" );
    }

    adminMode_displayWaitingMsg();
  }

}

/**
   Delete all logs
*/
void adminMode_deleteAll() {

  if ( adminMode_checkCardAvailable() ) {

    adminMode_displayMsg( "DELETEALL" );

    adminMode_sendRespStart();

    // Just scan the root directory for log files
    File root = SD.open("/");
    File entry =  root.openNextFile();
    bool deleted = true;
    while ( (entry != NULL) && (deleted) ) {

      
      if ( !entry.isDirectory() ) {
        String logName = String(entry.name());
        if ( SD.remove(logName.c_str()) ) {
          logName.concat(" DELETED");
          deleted = true;
          // Rewind to get the next available file
          root.rewindDirectory();
        } else {
          logName.concat(" NOT DELETED");
          deleted = false;
        }
        adminMode_sendRespLine( (char *) logName.c_str() );
      }
      entry =  root.openNextFile();
    }

    adminMode_sendRespEnd();

    adminMode_displayWaitingMsg();
  }

}

void adminMode_ErrorHandler() {
  adminMode_sendRespError( "COMMAND NOT RECOGNISED" );
}

/**
   Checks if SD card is available if not sends an error response
*/
bool adminMode_checkCardAvailable() {

  if ( sdCardAvailable ) {
    return true;
  } else {
    adminMode_sendRespError( "NO CARD" );
    return false;
  }
}

/**
   Indicate start of a response
*/
void adminMode_sendRespStart() {
  adminMode_sendRespLine( "<<<" );
}

/**
   Indicate end of a response
*/
void adminMode_sendRespEnd() {
  adminMode_sendRespLine( ">>>" );
}

/**
   Indicate an error response
*/
void adminMode_sendRespError(char *pMsg) {
  adminMode_sendRespLine( "<<<ERROR" );
  adminMode_sendRespLine( pMsg );
  adminMode_sendRespLine( ">>>" );
}

/**
   Send response and line terminator
*/
void adminMode_sendRespLine(char *pLine) {
  Serial.print( pLine );
  adminMode_sendRespTerm();
}

/**
   Send partial response, no terminator
*/
void adminMode_sendResp(char *pLine) {
  Serial.print( pLine );
}

/**
   Send response line terminator
*/
void adminMode_sendRespTerm() {
  Serial.print( "\r\n" );
}

/**
   Display waiting message
*/
void adminMode_displayWaitingMsg() {

  adminMode_displayMsg("WAITING...");
}

/**
   Display message on second line
*/
void adminMode_displayMsg(char *pMsg) {

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println( "ADMIN MODE" );
  display.println( pMsg );
  display.display();
}
