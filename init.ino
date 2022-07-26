/**
 * Initialisation functions
 */

void commonSetup() {

  // Mode button
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  
  // LEDs
  initialiseLEDs();

  // Initialise Display
  initialiseDisplay();

  // Initialise SD card
  initialiseSDCard();
}

/**
 * Configure LEDs
 */
void initialiseLEDs() {
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

}

/**
 * Configure and clear display
 */
void initialiseDisplay() {
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
}

/**
 * Configure GPS
 */
void initialiseGps() {
  
  gps.begin(9600);
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  gps.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPS_SERIAL.println(PMTK_Q_RELEASE);
}

/**
 * Configure SD card
 */
void initialiseSDCard() {
  sdCardAvailable = SD.begin(SD_CARD);
}
