#include "Freenove_WS2812B_RGBLED_Controller.h"
#define I2C_ADDRESS  0x20
#define LEDS_COUNT   10  //it defines number of lEDs. 

#define PIN_SERVO      2
#define PIN_DIRECTION_LEFT  4
#define PIN_DIRECTION_RIGHT 3
#define PIN_MOTOR_PWM_LEFT  6
#define PIN_MOTOR_PWM_RIGHT 5
#define PIN_SONIC_TRIG    7
#define PIN_SONIC_ECHO    8
#define PIN_IRREMOTE_RECV 9
#define PIN_SPI_CE      9
#define PIN_SPI_CSN     10
#define PIN_SPI_MOSI    11
#define PIN_SPI_MISO    12
#define PIN_SPI_SCK     13
#define PIN_BATTERY     A0
#define PIN_BUZZER      A0
#define PIN_TRACKING_LEFT A3
#define PIN_TRACKING_CENTER A2
#define PIN_TRACKING_RIGHT  A1
#define MOTOR_PWM_DEAD    10
#define MAX_DISTANCE 20

#define TK_STOP_SPEED          0
#define TK_FORWARD_SPEED        (100 + tk_VoltageCompensationToSpeed    )

//define different speed levels
int tk_VoltageCompensationToSpeed;  //define Voltage Speed Compensation
#define TK_TURN_SPEED_LV4       (160 + tk_VoltageCompensationToSpeed   )
#define TK_TURN_SPEED_LV3       (130 + tk_VoltageCompensationToSpeed   )
#define TK_TURN_SPEED_LV2       (-120 + tk_VoltageCompensationToSpeed  )
#define TK_TURN_SPEED_LV1       (-140 + tk_VoltageCompensationToSpeed  )

float batteryVoltage = 0;
bool isBuzzered = false;

NewPing sonar(PIN_SONIC_TRIG, PIN_SONIC_ECHO, MAX_DISTANCE);
float duration, distance;

Freenove_WS2812B_Controller strip(I2C_ADDRESS, LEDS_COUNT, TYPE_GRB); //initialization

void setup() {
  pinsSetup(); //set up pins
  getTrackingSensorVal();//Calculate Voltage speed Compensation
  while (!strip.begin());
  Serial.begin(9600);
}
void loop() {
  u8 trackingSensorVal = 0;
  trackingSensorVal = getTrackingSensorVal(); //get sensor value

  switch (trackingSensorVal)
  {
    case 0: strip.setAllLedsColor(255, 255, 0); //Set all LED color to yellow  //000
      motorRun(TK_STOP_SPEED, TK_STOP_SPEED); 
      delay(5000); 
      motorRun(TK_FORWARD_SPEED, TK_FORWARD_SPEED);
      delay(500);//car move forward
      break;
    case 7: strip.setAllLedsColor(0xFF0000); //set all LED color to red //111
      motorRun(TK_STOP_SPEED, TK_STOP_SPEED); //car stop
      break;
    case 1: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //001
      motorRun(TK_TURN_SPEED_LV4, TK_TURN_SPEED_LV1); //car turn
      break;
    case 3: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //011
      motorRun(TK_TURN_SPEED_LV3, TK_TURN_SPEED_LV2); //car turn right
      break;
    case 2: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //010
    case 5: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //101
      motorRun(TK_FORWARD_SPEED, TK_FORWARD_SPEED);  //car move forward
      break;
    case 6: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //110
      motorRun(TK_TURN_SPEED_LV2, TK_TURN_SPEED_LV3); //car turn left
      break;
    case 4: strip.setAllLedsColor(0x00FF00); //set all LED color to gree  //100
      motorRun(TK_TURN_SPEED_LV1, TK_TURN_SPEED_LV4); //car turn right
      break;
    default:
      break;
  }
  
}

void tk_CalculateVoltageCompensation() {
  getBatteryVoltage();
  float voltageOffset = 7 - batteryVoltage;
  tk_VoltageCompensationToSpeed = 30 * voltageOffset;
}

//when black line on one side is detected, the value of the side will be 0, or the value is 1
u8 getTrackingSensorVal() {
  u8 trackingSensorVal = 0;
  trackingSensorVal = (digitalRead(PIN_TRACKING_LEFT) == 1 ? 1 : 0) << 2 | (digitalRead(PIN_TRACKING_CENTER) == 1 ? 1 : 0) << 1 | (digitalRead(PIN_TRACKING_RIGHT) == 1 ? 1 : 0) << 0;
  return trackingSensorVal;
}

void pinsSetup() {
  //define motor pin
  pinMode(PIN_DIRECTION_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_DIRECTION_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);
  //define ultrasonic moduel pin
  pinMode(PIN_SONIC_TRIG, OUTPUT);
  pinMode(PIN_SONIC_ECHO, INPUT);
  //define tracking sensor pin
  pinMode(PIN_TRACKING_LEFT, INPUT);
  pinMode(PIN_TRACKING_RIGHT, INPUT);
  pinMode(PIN_TRACKING_CENTER, INPUT);
  setBuzzer(false);
}

void motorRun(int speedl, int speedr) {
  int dirL = 0, dirR = 0;
  if (speedl > 0) {
    dirL = 0;
  }
  else {
    dirL = 1;
    speedl = -speedl;
  }
  if (speedr > 0) {
    dirR = 1;
  }
  else {
    dirR = 0;
    speedr = -speedr;
  }
  speedl = constrain(speedl, 0, 255); // speedl absolute value should be within 0~255
  speedr = constrain(speedr, 0, 255); // speedr absolute value should be within 0~255
  digitalWrite(PIN_DIRECTION_LEFT, dirL);
  digitalWrite(PIN_DIRECTION_RIGHT, dirR);
  analogWrite(PIN_MOTOR_PWM_LEFT, speedl);
  analogWrite(PIN_MOTOR_PWM_RIGHT, speedr);
}

bool getBatteryVoltage() {
  if (!isBuzzered) {
    pinMode(PIN_BATTERY, INPUT);
    int batteryADC = analogRead(PIN_BATTERY);
    if (batteryADC < 614)    // 3V/12V ,Voltage read: <2.1V/8.4V
    {
      batteryVoltage = batteryADC / 1023.0 * 5.0 * 4;
      return true;
    }
  }
  return false;
}

void setBuzzer(bool flag) {
  isBuzzered = flag;
  pinMode(PIN_BUZZER, flag);
  digitalWrite(PIN_BUZZER, flag);
}

void alarm(u8 beat, u8 repeat) {
  beat = constrain(beat, 1, 9);
  repeat = constrain(repeat, 1, 255);
  for (int j = 0; j < repeat; j++) {
    for (int i = 0; i < beat; i++) {
      setBuzzer(true);
      delay(100);
      setBuzzer(false);
      delay(100);
    }
    delay(500);
  }
}

void resetCarAction() {
  motorRun(0, 0);
  setBuzzer(false);
}
void LED() {
  for (int k = 0; k < 255; k = k + 2) {
    strip.setAllLedsColorData(strip.Wheel(k)); // set color data for all LED
    strip.show();                              // show the color set before
    delay(50);
  }
  delay(3000);

  for (int j = 0; j < 255; j += 2) {
    for (int i = 0; i < LEDS_COUNT; i++) {
      strip.setLedColorData(i, strip.Wheel(i * 256 / LEDS_COUNT + j)); //set color data for LED one by one
    }
    strip.show();  // show the color set
    delay(50);
  }
}
