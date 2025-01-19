/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 */

int Check_Remaining_Moves() {
  // check if any of the stepper should be moved,
  // if yes then return 1 otherwise 0
  int result = 0;
  for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
    for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
      // adjust the active rotation matrix
      if (active_rotation[panel_nr][motor_nr] > 0) {
        result = 1;
        break; // if at least one of the stepper should move we don't need to continue
      }
    }  
  }
  return result;
}

void MoveSteppers() {
  // move only these stepper motors that are required to move
    bool local_required_pin_state = 0; // a local help parameter
  // local help parameter for the direction definition
    int local_dir = 0;
  // move all the steppers at once "nr_steps" microsteps 
  for (int state = 0; state < nr_steps; state++) { // go through all the micro steps
    // start writing
    digitalWrite(LAT_Pin, LOW);

    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) { // go through all the motors
      // go through all the stepper pins 
      for (int i = 0; i < 4; i++) {
        // set shift pin to "wait"
        digitalWrite(CLK_Pin, LOW);
        // move only if the motor should be moved
        for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
          // get the direction
            local_dir = active_direction[panel_nr][motor_nr];
          // define the pin state
            local_required_pin_state = 0;
            if (active_rotation[panel_nr][motor_nr] > 0) local_required_pin_state = step_states[local_dir][state][i];
          // write the pin state
            digitalWrite(Data_Pin[panel_nr], local_required_pin_state);
        }
        // rising slope -> shifting data in the register
        digitalWrite(CLK_Pin, HIGH);
      }   
    }

    // write whole register to output
    digitalWrite(LAT_Pin, HIGH); 
    // required delay between the steps
    delay(step_delay); // should be step_delay = 2 
  }
}

void AdjustRemainingMoves() {
  // adjust the active_rotation matrix
  // after every movement reduce the active moves by 1
  for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
    for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
      // adjust the active rotation matrix
      if (active_rotation[panel_nr][motor_nr] > 0) active_rotation[panel_nr][motor_nr] -= 1;
    }  
  }
}

void ActiveStepperMovement() {
  int should_move = 0;
  should_move = Check_Remaining_Moves(); // check if any of the stepper should be moved
  while (should_move == 1) {
    // move steppers only if there are any movements left
    MoveSteppers();
    AdjustRemainingMoves(); 
    should_move = Check_Remaining_Moves(); // check if any of the stepper should be moved
  }
  AllSteppersOff(); // turn all steppers OFF
}

void Homing_Check_position() {
  //Serial.println("Check_position()");
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // do something only if new is different from now
      if (position_now[panel_nr][motor_nr] != position_new[panel_nr][motor_nr]) {
        // set the direction
          active_direction[panel_nr][motor_nr] = 1;
        // calculate how many moves are needed for the new position
          active_rotation[panel_nr][motor_nr] = rotation_micro_steps;
        // lets add some extra, to be sure that we are at home
           active_rotation[panel_nr][motor_nr] += 5;
        // just to be sure
          if (active_rotation[panel_nr][motor_nr] < 0)                    active_rotation[panel_nr][motor_nr] = 0;
          if (active_rotation[panel_nr][motor_nr] > rotation_micro_steps + 5) active_rotation[panel_nr][motor_nr] = rotation_micro_steps + 5;
      }
    } 
  } 
}

void Homing_set_pos_new(){
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // adjust the now matrix
      position_new[panel_nr][motor_nr] = 0;
    }
  }  
}

void Homing() {
  // write it to serial
    Serial.println("homing started");
  // adjust the NEW matrix
    Homing_set_pos_new();
  // check whitch dials must be moved
    Homing_Check_position();
  // move all the necessary steppers    
    ActiveStepperMovement();
  // turn everything off
    AllSteppersOff();
  // adjust the NOW matrix
    Homing_set_pos_now();
  // write it to serial
    Serial.println("homing done");
}

void Check_position() {
  //Serial.println("Check_position()");
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // do something only if NEW is different from NOW
      if (position_now[panel_nr][motor_nr] != position_new[panel_nr][motor_nr]) {
        // check which segment must be moved
          active_rotation[panel_nr][motor_nr] = (position_new[panel_nr][motor_nr] - position_now[panel_nr][motor_nr]) * rotation_micro_steps;
        // set the direction
          active_direction[panel_nr][motor_nr] = 0;
        // check if we need to move opposite direction
          if (active_rotation[panel_nr][motor_nr] < 0) {
            // if the step number is negative we need to rotate to oposite direction, thus direction change
              active_direction[panel_nr][motor_nr] = 1;
            // number of steps must be positive
              active_rotation[panel_nr][motor_nr] *= -1;
          }
        
        // just to be sure
          if (active_rotation[panel_nr][motor_nr] < 0)                    active_rotation[panel_nr][motor_nr] = 0;
          if (active_rotation[panel_nr][motor_nr] > rotation_micro_steps) active_rotation[panel_nr][motor_nr] = rotation_micro_steps;
      }
    }
  } 
}

void new_to_now_matrix() {
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // adjust the now matrix
      position_now[panel_nr][motor_nr] = position_new[panel_nr][motor_nr];
    } 
  }   
}

void convert_Panel_Pos_to_Segments() {
  // covert the panel positions to segment positions
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // adjust the new matrix
      position_new[panel_nr][motor_nr] = symbol_pos[panel_positions[panel_nr]][motor_nr];
    } 
  } 
}

void start_demo() {
  for (int i = 0; i < 10; i++) {
    Serial.print("Demo -> "); Serial.println(i);
    // adjust the now matrix
      panel_positions[3] = i;
      panel_positions[2] = i;
      panel_positions[1] = i;
      panel_positions[0] = i;
      
    // convert the Panel positions to Segment positions
      convert_Panel_Pos_to_Segments();    
    // check whitch Segments must be moved
      Check_position();
    // move all the necessary steppers    
      ActiveStepperMovement();   
    // turn everything off
      AllSteppersOff(); 
    // adjust now matrix
      new_to_now_matrix(); 
    // delay
      delay(5000);
  }   

  Serial.print("Demo -> OFF");
  // adjust the now matrix
    panel_positions[3] = 25;
    panel_positions[2] = 25;
    panel_positions[1] = 25;
    panel_positions[0] = 25;
    
  // convert the Panel positions to Segment positions
    convert_Panel_Pos_to_Segments();    
  // check whitch Segments must be moved
    Check_position();
  // move all the necessary steppers    
    ActiveStepperMovement();   
  // turn everything off
    AllSteppersOff(); 
  // adjust now matrix
    new_to_now_matrix(); 
}
