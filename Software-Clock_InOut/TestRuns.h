/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 */

void ULN_Blink(){
  // blink all the LEDs at the same time
  AllPinsON();
  delay(500);
  AllSteppersOff();
  delay(500);
}

void move_all_steppers_8() {

  for (int j = 0; j < 8; j++) {
    MoveAllSteppers(); // --> it already makes "nr_steps" micro-steps
  }
  AllSteppersOff();

  Serial.println("moved 8 steps");
}
