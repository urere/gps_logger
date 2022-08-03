
/**
 * Start GPX file
 */
void gpx_writeStart( char *pName ) {

  Serial.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  Serial.println("<gpx version=\"1.0\" xmlns=\"http://www.topografix.com/GPX/1/0\">");
  Serial.println("<trk>");
  Serial.print("<name>");
  Serial.print(pName);
  Serial.println("</name>");
  Serial.println("<trkseg>");
  
}

/**
 * Finished GPX file
 */
void gpx_writeEnd() {
  
  Serial.println("</trkseg>");
  Serial.println("</trk>");
  Serial.println("</gpx>");
}

/**
 * Add track point to GPX file
 */
void gpx_writeTrackPoint( char* pTs, nmea_float_t latitude, nmea_float_t longitude, nmea_float_t speed, nmea_float_t course ) {
  
  Serial.print("<trkpt lat=\"");
  Serial.print( latitude, 10 );
  Serial.print("\" lon=\"");
  Serial.print( longitude, 10 );
  Serial.println("\">");

  Serial.print("<time>");
  Serial.print( pTs );
  Serial.println("</time>");

  Serial.print("<course>");
  Serial.print( course, 6 );
  Serial.println("</course>");

  Serial.print("<speed>");
  // Convert to m/s
  Serial.print( speed * 0.514444, 6 );
  Serial.println("</speed>");
  
  Serial.println("</trkpt>");
}

/**
 * Finished GPX file
 */
void gpx_writeParseErrorComment( const char *pLogLine ) {
  
  Serial.print("<!-- PARSE ERROR: [");
  Serial.print(pLogLine);
  Serial.println("] -->");
}
