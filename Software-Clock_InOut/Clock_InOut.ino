/*  Clock-In-Out v1 
 * 
 * Dr. Tõnis
 * Pfäffikon 2025
 * 
 */

// OTA libraries and parameters
  #include "WiFi.h"
// WiFi Login Data
  #include "credentials.h" // in the libraries folder add credentials/credentials.h with the ssid and pw
  // main wifi
    const char *ssid1     = my_ssid1;
    const char *password1 = my_password1;
  // backup network
    const char *ssid2     = my_ssid2;
    const char *password2 = my_password2;
// time libraries and functions
  #include "time.h"                      // time()
  #define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"        // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  int old_minute = 0;                    // help parameter to save the last minute value
  unsigned long previousMillis = 0;      // will store last time the time was updated
  const long interval = 10000;           // interval at which to check time
    
// Motor Pins
  const int CLK_Pin   = D6; // connected to SHCP
  const int LAT_Pin   = D4; // connected to STCP
  const int Data_Pin[4] = {D10, D7, D8, D3}; // connected to DS (data in3) {H1(-H13-H1-H12), H2(-H12), H3 (pico), H14(-H12), H15(-H1-H13-H12)}

// Motor movement parameters
  const int nr_steps = 8; // 8 or 4 steps per movement -> hard coded 8, thus don't change!
  // micro stepping
  const int step_states[2][8][4] = {{{0, 0, 0, 1}, // forwards
                                     {0, 0, 1, 1},
                                     {0, 0, 1, 0},
                                     {0, 1, 1, 0},
                                     {0, 1, 0, 0},
                                     {1, 1, 0, 0},
                                     {1, 0, 0, 0},
                                     {1, 0, 0, 1}}, 
                                    {{1, 0, 0, 1}, // backwards
                                     {1, 0, 0, 0},
                                     {1, 1, 0, 0},
                                     {0, 1, 0, 0},
                                     {0, 1, 1, 0},
                                     {0, 0, 1, 0},
                                     {0, 0, 1, 1},
                                     {0, 0, 0, 1}}};                              

// configuration  
  #define nr_panels 4 // how many dial panels do we have - one panel has 12 digits                     
  #define nr_steppers 7 // number of steppers connected in series to the shift registers (defined by the PCB)
  const int step_delay = 2; // delay between steps, min 2. the higher values mean slower movement
  const int rotation_micro_steps = 1024; // how many steps one should move
                                                           
  int active_rotation[nr_panels][nr_steppers] =           {{0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0}}; // how many steps should we move

  int position_now[nr_panels][nr_steppers] =              {{0, 0, 0, 0, 0, 0, 0}, // 0 - home / 1 - out
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0}}; // in which position are we now

  int position_new[nr_panels][nr_steppers] =              {{0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0}}; // the next position should be that one
  int active_direction[nr_panels][nr_steppers] =          {{0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0},
                                                           {0, 0, 0, 0, 0, 0, 0}}; // in which direction we should turn

// possible symbols {g, f, e, d, c, b, a, empty} // {J1, J2, J3, J4, J5, J6, J7, empty}
  const bool symbol_pos[26][nr_steppers + 1] = {{0, 1, 1, 1, 1, 1, 1, 0}, // 0
                                                {0, 0, 0, 0, 1, 1, 0, 0}, // 1
                                                {1, 0, 1, 1, 0, 1, 1, 0}, // 2
                                                {1, 0, 0, 1, 1, 1, 1, 0}, // 3
                                                {1, 1, 0, 0, 1, 1, 0, 0}, // 4
                                                {1, 1, 0, 1, 1, 0, 1, 0}, // 5
                                                {1, 1, 1, 1, 1, 0, 0, 0}, // 6
                                                {0, 0, 0, 0, 1, 1, 1, 0}, // 7
                                                {1, 1, 1, 1, 1, 1, 1, 0}, // 8
                                                {1, 1, 0, 0, 1, 1, 1, 0}, // 9
                                                
                                                {1, 1, 1, 0, 1, 1, 1, 0}, // A - 10
                                                {0, 1, 1, 1, 0, 0, 1, 0}, // C - 11
                                                {1, 1, 1, 0, 0, 0, 1, 0}, // E - 12
                                                {1, 1, 1, 0, 0, 0, 1, 0}, // F - 13
                                                {0, 1, 1, 1, 1, 0, 1, 0}, // G - 14
                                                {1, 1, 1, 0, 1, 1, 0, 0}, // H - 15
                                                {0, 0, 0, 1, 1, 1, 0, 0}, // J - 16
                                                {0, 1, 1, 1, 0, 0, 0, 0}, // L - 17
                                                {1, 1, 1, 0, 0, 1, 1, 0}, // P - 18
                                                {0, 0, 0, 0, 1, 1, 1, 0}, // T - 19
                                                {0, 1, 1, 1, 1, 1, 0, 0}, // U - 20
                                                {1, 0, 1, 1, 1, 0, 0, 0}, // o - 21
                                                {1, 1, 0, 0, 0, 1, 1, 0}, // deg symb - 22
                                                {1, 0, 0, 0, 0, 0, 0, 0}, // -        - 23
                                                {0, 0, 0, 1, 0, 0, 0, 0}, // _        - 24
                                                {0, 0, 0, 0, 0, 0, 0, 0}}; // off     - 25

  int panel_positions[nr_panels] = {0, 3, 8, 1}; // panel_positions[0] is the far right and panel_positions[3] is the far left one -> 18:30
  
// Serial reading parameters
  String inputString = "";          // a String to hold incoming data
  bool stringComplete = false;      // whether the string is complete
  int new_state = 0;

// include other functions
  #include "Move_Basics.h"
  #include "Move_Active.h"
  #include "TestRuns.h" // not needed, only for testing purposes  
  #include "Initialization.h"
  #include "SerialFunctions.h"
  #include "Time_functions.h"

void setup() {
  // initiate all the pins
    setup_init();
  // connect to the WiFi
    setup_wifi();
  // turn all Steppers Off
    AllSteppersOff();
  // connect time server
    init_time_server(MY_TZ);    
  // homing
    Homing_boot();

  // demo
    start_demo();
}

void loop () {
  // check if there are some new command in Serial
    SerialCommands();
  // check the passed time  
    unsigned long currentMillis = millis();
  // continue only if enough time has passed
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
        previousMillis = currentMillis;
      // show the current time
        get_time_now();
      // check if we need to change the time
        check_time_change_minute();  
    }
}
