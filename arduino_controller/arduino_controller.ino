/* 
File: arduino_controller.ino

This file contains the top level control code for just using and actuating on the incoming packets.
It also manages a state machine to move between the following:
OFF -> Wheelchair is off, nothing to do
WAKEUP -> Wheelchair takes some time to initialize once started. Wait this out in this state
IDLE -> Wheelchair on, but nothing to do, is still
RUNNING -> Wheelchair moving

The embedded interface consumes a single byte for each speed and direction (0 -> 255)
We then center it about 127 to get -127 -> 128.
Since the wheelchair expects 6V to be neutral (not moving), we must re-center around that.
By adding the -127->128 onto 255*0.6 = 153 to get a range of 26 -> 281, then clamped to a set percentage.
By default, this percentage is MAX_PERCENTAGE and MIN_PERCENTAGE set to 48% and 72%. 
*/

#include "input_jetson.hpp"

// Operational Parameters
#define MAX_PERCENTAGE 0.72f
#define MIN_PERCENTAGE 0.48f
#define IDLE_TIMEOUT_MS  3000U
#define WAKEUP_TIMEOUT_MS 7000U

// Output / Input Levels
const int output_neutral = int(255.0f * 0.6f); // 6V output_neutral for wheelchair
const int input_midpoint = 255 / 2; // 5V input_midpoint
const int minimum = int(255.0f * MIN_PERCENTAGE);
const int maximum = int(255.0f * MAX_PERCENTAGE);

typedef const int PIN;
typedef enum {
  OFF = 0,
  WAKEUP = 1,
  RUNNING = 2,
  IDLE = 3,
  NUM_STATES,
} state;

// GPIO Pings
PIN wakeUpPin = 12;
PIN ledPin = 13;

// Logic Variables
state cur_state = OFF;
state next_state = OFF;
bool newCommand = false;
int wakeup = LOW;
int speed_val = 0;
int dir_val = 0;
int deflections[2] = {input_midpoint, input_midpoint};

unsigned long wakeup_time = 0;
unsigned long last_cmd_time = 0;

void set_neutral() {
  deflections[SPEED] = input_midpoint;
  deflections[DIRECTION] = input_midpoint;
  OCR2B = output_neutral;
  OCR2A = output_neutral;
}

int clamp(int val) {
  return min( max(val, minimum), maximum);
}

void acknowledge()
{
  Serial.print(deflections[SPEED]);
  Serial.print(",");
  Serial.print(deflections[DIRECTION]);
  Serial.print(",");
  Serial.print(cur_state);
  Serial.println();
}


void setup()
{
  // Setup Serial port
  setup_jetson_serial();
  delay(1500);

  // Configure Hardware PWMs
  pinMode(3, OUTPUT); // PWM pin3, not analog pin 3 (using digital for pwm)
  pinMode(11, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(wakeUpPin, INPUT);
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  set_neutral();

  // Wipe state of the buffer from Arduino's side
  while(Serial.available() > 0) {
    Serial.read();
  }

  // Tell jetson we are awake
  Serial.println("AWAKE");
}

void loop()
{
  newCommand = get_deflections_from_jetson(deflections);
  if (newCommand) {
    acknowledge();
  }

  wakeup = digitalRead(wakeUpPin);
  if (wakeup == LOW) {
    cur_state = OFF;
  }

  next_state = cur_state;
  switch(cur_state) {
    case OFF:
      set_neutral();
      // Set LEDs to off
      digitalWrite(ledPin, LOW);
      if (wakeup) {
        next_state = WAKEUP;
        wakeup_time = millis();
      }
      break;

    case WAKEUP:
      set_neutral();
      digitalWrite(ledPin, LOW);
      if ((millis() - wakeup_time) > WAKEUP_TIMEOUT_MS) {
        if (newCommand) { // Go to running if we already have a new command
          next_state = RUNNING;
        } else {
          next_state = IDLE;
        }
      }
      break;

    case IDLE:
      digitalWrite(ledPin, HIGH);
      set_neutral();
      if (newCommand) {
        next_state = RUNNING;
        last_cmd_time = millis();
      }
      break;

    case RUNNING:
      digitalWrite(ledPin, HIGH);
      if (newCommand) {
        speed_val = deflections[SPEED] - input_midpoint; // -125 to 125 of 0 - 5V
        dir_val = deflections[DIRECTION] - input_midpoint;
        OCR2A = clamp(output_neutral + dir_val);
        OCR2B = clamp(output_neutral + speed_val);
        last_cmd_time = millis();
        next_state = RUNNING;
      } else if ((millis() - last_cmd_time) > IDLE_TIMEOUT_MS) {
        // Serial.println("MOVING TO IDLE");
        next_state = IDLE;
      } else {
      }
      break;

    case NUM_STATES:
    default:
      break;
  }

  cur_state = next_state;
}
