/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 */
void write2serial_pos_now() {
  Serial.println("Panel position_NOW "); 
  Serial.print("  ");
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    Serial.print(panel_positions[panel_nr]); Serial.print("  ");
  }
  Serial.println(""); 
  Serial.println("Segment position_NOW "); 
  // go through each dial, panel by panel and indicate the current position
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    Serial.print("  ");
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      Serial.print(position_now[panel_nr][motor_nr]);
      if (motor_nr < nr_steppers - 1) Serial.print(", ");
      else Serial.println("");
    }  
  }    
}

void write2serial_pos_new() {
  Serial.println("Panel position_NEW "); 
  Serial.print("  ");
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    Serial.print(panel_positions[panel_nr]); Serial.print("  ");
  }
  Serial.println(""); 
  Serial.println("Segment position_NEW "); 
  // go through each dial, panel by panel and indicate the current position
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    Serial.print("  ");
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      Serial.print(position_new[panel_nr][motor_nr]);
      if (motor_nr < nr_steppers - 1) Serial.print(", ");
      else Serial.println("");
    }  
  }  
}

void UnknownCommand() {
  // write to Serial that the Command is unknown
    Serial.println("Unknow command!");
    Serial.println("Use:");
    Serial.println(" mm - move all digits \"nr_steps\" microsteps");
    Serial.println(" hh - perform homing");
    Serial.println(" bh - perform Homing_boot()");
    Serial.println(" bb - blink all the ULN LEDs");
    Serial.println(" po - nOw positions matrix");
    Serial.println(" pe - nEw positions matrix");

    Serial.println(" nsX - set all Segments to the same position (0-1)");
    Serial.println(" npX - set all PANELS to the same position (0-25)");
    Serial.println(""); 
}

void GetNewSegmentPosition() {
  // define new Dial position
    Serial.println(""); Serial.println(inputString);
  // local help paramter
    int cont_serial = 0;
  // check if all the characters are numbers
  for (int i = 2; i <= inputString.length() - 1; i++) {
    // if not all the characters are numbers we do not continue
      if (isdigit(inputString[i])) cont_serial = 1;
      else UnknownCommand();
  }
  
  // only adjust the position when we have a number in the code
    if (cont_serial == 1) {
      // define local parameter
        String number = "";
      // get the number
        for (int i = 2; i <= inputString.length() - 1; i++) {
          number += inputString[i];
        }
      // convert String to int
        int dial_nr_new = number.toInt(); // convert string to int
      // write the new dial position to serial
        Serial.print("New Segment Position is "); Serial.println(dial_nr_new);  
      // check if it's a valid Nr
        if (dial_nr_new >= 0 && dial_nr_new < 2) {
          // go through each dial, panel by panel and indicate the current position
            for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
              for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
                position_new[panel_nr][motor_nr] = dial_nr_new;
              }  
            } 
          // write the position matrix to serial
            write2serial_pos_new();
        }
        else UnknownCommand();
    }
  
  else UnknownCommand();
}

void GetNewPanelPosition() {
  // define new Dial position
    Serial.println(""); Serial.println(inputString);
  // local help paramter
    int cont_serial = 0;
  // check if all the characters are numbers
  for (int i = 2; i <= inputString.length() - 1; i++) {
    // if not all the characters are numbers we do not continue
      if (isdigit(inputString[i])) cont_serial = 1;
      else UnknownCommand();
  }
  // only adjust the position when we have a number in the code
    if (cont_serial == 1) {
      // define local parameter
        String number = "";
      // get the number
        for (int i = 2; i <= inputString.length() - 1; i++) {
          number += inputString[i];
        }
      // convert String to int
        int dial_nr_new = number.toInt(); // convert string to int
      // write the new dial position to serial
        Serial.print("New Panel Position is "); Serial.println(dial_nr_new);  
      // check if it's a valid Nr
        if (dial_nr_new >= 0 && dial_nr_new < 26) {
          // go through each panel, panel by panel and indicate the current position
            for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
              panel_positions[panel_nr] = dial_nr_new;
            } 
        }
        else UnknownCommand();
    }
  
  else UnknownCommand();
}

void SerialCommands() {
  // check the command when a newline arrives:
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\r');
    if (inputString.length() >= 2) {
      //Serial.println(inputString);
      if      ((inputString[0] == 'm') && (inputString[1] == 'm')) move_all_steppers_8();  // move all the digits one bit further
      else if ((inputString[0] == 'h') && (inputString[1] == 'h')) Homing();               // perform homing
      else if ((inputString[0] == 'b') && (inputString[1] == 'h')) Homing_boot();          // perform Homing_boot
      else if ((inputString[0] == 'b') && (inputString[1] == 'b')) ULN_Blink();            // blink all the ULN LEDs
      else if ((inputString[0] == 'p') && (inputString[1] == 'o')) write2serial_pos_now(); // write to serial the NOW positions
      else if ((inputString[0] == 'p') && (inputString[1] == 'e')) write2serial_pos_new(); // write to serial the NEW positions

      else if ((inputString.length() >= 3) && (isdigit(inputString[2]))) { // the string is long enough and the 3rd position is a number
        if ((inputString[0] == 'n') && (inputString[1] == 's')) {
          // get the new Segment position Nr
            GetNewSegmentPosition();     
          // check whitch Segments must be moved
            Check_position();
          // move all the necessary steppers    
            ActiveStepperMovement();   
          // turn everything off
            AllSteppersOff(); 
          // adjust now matrix
            new_to_now_matrix(); 
        }
        else if ((inputString.length() >= 3) && (isdigit(inputString[2]))) { // the string is long enough and the 3rd position is a number
          if ((inputString[0] == 'n') && (inputString[1] == 'p')) {
            // get the new Panel position Nr
              GetNewPanelPosition(); 
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
      }
      else UnknownCommand(); // if the command is unknown we inform the user
      
      // clear the string:
        inputString = "";
    }
  }
}
