/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 */

void AllSteppersOff() {
  // turn all the shift register outputs LOW
  // start writing
  digitalWrite(LAT_Pin, LOW);
  // go through all the stepper pins
  for (int i = 0; i < nr_steppers * 4; i++) {
    // set shift pin to "wait"
    digitalWrite(CLK_Pin, LOW);
    // writing to data pin
    for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
      // move steppers on all the panels at the same time
      digitalWrite(Data_Pin[panel_nr], LOW); // output pin state
    }
    // rising slope -> shifting data in the register
    digitalWrite(CLK_Pin, HIGH);
  }
  // write whole register to output
  digitalWrite(LAT_Pin, HIGH);  
}

void AllPinsON() {
  // turn all the shift register outputs LOW
  
  // start writing
  digitalWrite(LAT_Pin, LOW);
  // go through all the stepper pins
  for (int i = 0; i < nr_steppers * 4; i++) {
    // set shift pin to "wait"
    digitalWrite(CLK_Pin, LOW);
    // writing to data pin
    for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
      // move steppers on all the panels at the same time
      digitalWrite(Data_Pin[panel_nr], HIGH); // output pin state
    }
    // rising slope -> shifting data in the register
    digitalWrite(CLK_Pin, HIGH);
  }
  // write whole register to output
  digitalWrite(LAT_Pin, HIGH);  
}

void MoveAllSteppers() {
  // local help parameter for the direction definition
    int local_dir = 1;
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
          digitalWrite(Data_Pin[panel_nr], step_states[local_dir][state][i % 4]); // output pin state
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

void Homing_set_pos_now(){
  for (int panel_nr = 0; panel_nr < nr_panels; panel_nr++) {
    for (int motor_nr = 0; motor_nr < nr_steppers; motor_nr++) {
      // adjust the now matrix
      position_now[panel_nr][motor_nr] = 0;
    }
    panel_positions[panel_nr] = 0;  
  }  
}

void Homing_boot() {
  // write it to serial
    Serial.println("homing @ boot started");
  // move all the steppers
    for (int pulse = 0; pulse < rotation_micro_steps; pulse++) {
      MoveAllSteppers();
    }
  // turn everything off
    AllSteppersOff();
  // adjust the NOW matrix
    Homing_set_pos_now();
  // write it to serial
    Serial.println("homing @ boot done");
}
