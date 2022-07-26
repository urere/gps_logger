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
        adminMode_sendRespLine( entry.name() );
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

      // Attempt to open the log file
      File logFile = SD.open(pArg);
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
      adminMode_sendRespStart();
      adminMode_sendRespLine( "DELETE command executed" );
      adminMode_sendResp( "  LOG:" );
      adminMode_sendRespLine( pArg );
      adminMode_sendRespEnd();
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
    adminMode_sendRespLine( "DELETEALL command executed" );
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
