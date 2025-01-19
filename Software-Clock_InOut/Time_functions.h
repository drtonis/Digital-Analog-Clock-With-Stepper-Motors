/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 * big help from https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/
*/


// needed for the following functions
  tm timeinfo;                              // the structure tm holds time information in a more convient way

void setTimezone(String timezone) {
  Serial.printf("  Setting Timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void init_time_server(String timezone) {
  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)) {
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}

void get_time_now() {
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time 1");
    return;
  }  
}

void check_time_change_minute() {
  // we do something if minute changes
  if (old_minute != timeinfo.tm_min) {
    // save the current minute value
      old_minute = timeinfo.tm_min;
    // adjust the values
      panel_positions[3] = int(timeinfo.tm_hour/10);
      panel_positions[2] = int(timeinfo.tm_hour%10);
      panel_positions[1] = int(timeinfo.tm_min/10);
      panel_positions[0] = int(timeinfo.tm_min%10);
      //Serial.println("new time");
      Serial.print(panel_positions[0]); Serial.print(panel_positions[1]); Serial.print(":"); Serial.print(panel_positions[2]); Serial.println(panel_positions[3]);
      
    // convert the Panel positions to Segment positions
      convert_Panel_Pos_to_Segments();    
    // write the position matrix to serial
      write2serial_pos_new();  
    // check whitch Segments must be moved
      Check_position();
    // move all the necessary steppers    
      ActiveStepperMovement();   
    // turn everything off
      AllSteppersOff(); 
    // adjust now matrix
      new_to_now_matrix();
  }
}
