#include <IRremote.h>
#include "RTClib.h"

/* remote buttons codes*/
#define B_BRIGHTNESS_UP 0xF700FF
#define B_BRIGHTNESS_DOWN 0xF7807F

#define B_POWER_OFF 0xF740BF
#define B_POWER_ON 0xF7C03F

#define B_RED 0xF720DF
#define B_GREEN 0xF7A05F
#define B_BLUE 0xF7609F
#define B_WHITE 0xF7E01F

#define B_RED_1 0xF710EF
#define B_GREEN_1 0xF7906F
#define B_BLUE_1 0xF750AF

#define B_RED_2 0xF730CF
#define B_GREEN_2 0xF7B04F
#define B_BLUE_2 0xF7708F

#define B_RED_3 0xF708F7
#define B_GREEN_3 0xF78877
#define B_BLUE_3 0xF748B7

#define B_RED_4 0xF728D7
#define B_GREEN_4 0xF7A857
#define B_BLUE_4 0xF76897

#define B_FLASH 0xF7D02F
#define B_STROBE 0xF7F00F
#define B_FADE 0xF7C837
#define B_SMOOTH 0xF7E817

/* Struct */
struct RGBW {
  byte r;
  byte g;
  byte b;
  byte w;
};

struct dateTime {
  int hour;
  int minute;
  int seconds;
};

/* PIN */
const int IR_PIN = 11;  // pin to the ir sensor
const int RedLed_PIN = 2; // pin to the red status led
const int GreenLed_PIN = 3; // pin to the green status led

const int Motor_PIN = 4;  //pin to the relay that controls the motor
const int Heater_PIN = 7; //pin to the relay that controls the heater

const int R_PIN = 10;  // pin to red channel of the strip
const int G_PIN = 9;  // pin to the green channel of the strip
const int B_PIN = 6;  // pin to the blue channel of the strip
const int W_PIN = 5;  // pin to the white channel

const int BUTTON_PIN = 12; // pin to the multi-action button

/*  VARIABLES */
bool lights_on = false; // defines if the lights are on
bool heater_on = false;  //defines if the heater is on
bool motor_on = false;  //defines if the motor is on

int brightness = 2; //general brightness (1-5)
byte mode = 0; // current mode of operation
/*  0 - standard mode: colors are selected by the user
    1 - 12h mode: light is controlled during the day and fully off in the night
    2 - 24h mode: light is controlled all day
*/

byte tmp_mode = 0;

unsigned long blink_delay_time = 0;
unsigned long hour_sync_time = 0;

RGBW current_color;
dateTime current_time = {0,0,0};

/*--------------------------------*/

IRrecv irrecv(IR_PIN);
decode_results results;
RTC_DS3231 rtc;

/* FUNCTIONS */
void IR_commands(int key){
  switch(key){
    case B_POWER_ON:
      lights_on = true;
      mode = 0; tmp_mode = mode;
      break;
    
    case B_POWER_OFF:
      lights_on = false;
      mode = 0; tmp_mode = mode;
      break;

    case B_BRIGHTNESS_UP:
      if (brightness < 5)
        brightness ++;
      mode = 0; tmp_mode = mode;
      break;
    
    case B_BRIGHTNESS_DOWN:
      if (brightness > 1)
        brightness --;
      mode = 0; tmp_mode = mode;
      break;
    
    case B_RED:
      current_color = {255, 0, 0, 0};
      mode = 0; tmp_mode = mode;
      break;

    case B_GREEN:
      current_color = {0, 255, 0, 0};
      mode = 0; tmp_mode = mode;
      break;

    case B_BLUE:
      current_color = {0, 0, 255, 0};
      mode = 0; tmp_mode = mode;
      break;

    case B_WHITE:  
    /* white color can be created in several ways
      - only white leds
      - rgb leds
      - white and rgb leds
    */
      if (current_color.r == 255 && current_color.g == 255 && current_color.b == 255)
        if (current_color.w == 255)
          current_color = {0, 0, 0, 255};
        else 
          current_color = {255,255,255,255};
      else
        if (current_color.w == 255)
          current_color = {255,255,255,0};
        else{
          current_color = {0,0,0,255};
        }
      mode = 0; tmp_mode = mode;
      break;
    
    case B_RED_1:
      current_color = {255,63,0,0};
      mode = 0; tmp_mode = mode;
      break;
    
    case B_RED_2:
      current_color = {255,127,0,0};
      mode = 0; tmp_mode = mode;
      break;
    
    case B_RED_3:
      current_color = {255,191,0,0};
      mode = 0; tmp_mode = mode;
      break;
    
    case B_RED_4:
      current_color = {255,255,0,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_GREEN_1:
      current_color = {0,255,63,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_GREEN_2:
      current_color = {0,255,127,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_GREEN_3:
      current_color = {0,255,191,0};
      mode = 0; tmp_mode = mode;
      break;
  
    case B_GREEN_4:
      current_color = {0,255,255,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_BLUE_1:
      current_color = {63,0,255,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_BLUE_2:
      current_color = {127,0,255,0};
      mode = 0; tmp_mode = mode;
      break;
    
    case B_BLUE_3:
      current_color = {191,0,255,0};
      mode = 0; tmp_mode = mode;
      break;
    
    case B_BLUE_4:
      current_color = {255,0,255,0};
      mode = 0; tmp_mode = mode;
      break;

    case B_FLASH:
    // button used to set the 12h mode
    // two clicks are needed to confirm
    // on first click the green led start blinking
      if (tmp_mode == 1){
        mode = 1;
        lights_on = true;
      }
      else
        tmp_mode = 1;
      break;

    case B_STROBE:
    // button used to set the 24h mode
    // two clicks are needed to confirm
    // on first click the green led start blinking
      if (tmp_mode == 2){
        mode = 2;
        lights_on = true;
      }
      else
        tmp_mode = 2;
      break;  
  }
}

ISR (PCINT0_vect){ // interrupt for the button   
     if (digitalRead(BUTTON_PIN) == HIGH){ //if the button is pressed
      mode = 0;
      tmp_mode = 0;
      lights_on = !lights_on;
      current_color = {255,255,255,0};
      brightness = 2;  
     }
}
/* END FUNCTIONS */

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver

  rtc.begin();

  *digitalPinToPCMSK(BUTTON_PIN) |= bit (digitalPinToPCMSKbit(BUTTON_PIN));
  PCIFR  |= bit (digitalPinToPCICRbit(BUTTON_PIN));
  PCICR  |= bit (digitalPinToPCICRbit(BUTTON_PIN));

  pinMode(GreenLed_PIN,OUTPUT);
  pinMode(RedLed_PIN,OUTPUT);

  pinMode(Motor_PIN,OUTPUT);
  pinMode(Heater_PIN,OUTPUT);

  pinMode(R_PIN,OUTPUT);
  pinMode(G_PIN,OUTPUT);
  pinMode(B_PIN,OUTPUT);
  pinMode(W_PIN,OUTPUT);

  if (rtc.lostPower()){
    current_time.hour = 0;
    current_time.minute = 0;
    current_time.seconds = 0;
  }
  else{
    DateTime now = rtc.now();

    current_time.hour = now.hour();
    current_time.minute = now.minute();
    current_time.seconds = now.second();
  }
  Serial.println(current_time.hour,DEC);
  Serial.println(current_time.minute,DEC);
  Serial.println(current_time.seconds,DEC);
  }

void loop() {
  if(millis() - hour_sync_time > 300000){
    // sync time 5 minutes

    if (rtc.lostPower()){
    current_time.hour = 0;
    current_time.minute = 0;
    current_time.seconds = 0;
    mode = 0;
    lights_on = false;
  }
  else{
    DateTime now = rtc.now();

    current_time.hour = now.hour();
    current_time.minute = now.minute();
    current_time.seconds = now.second();
  }
  }

  if (irrecv.decode(&results)) {
    IR_commands(results.value);
    Serial.println(results.value, HEX);
    irrecv.resume();
  }

  // if the user have pressed once the mode button -> start to blink green led
  if (mode != tmp_mode){
    if (millis() - blink_delay_time > 500){
      blink_delay_time = millis();
      digitalWrite(GreenLed_PIN, !digitalRead(GreenLed_PIN));
    }
  }
  else{
    if (mode == 0){
      // in standard mode green led is on when the light is on, red led is on when the lights are off
      if (lights_on){
        digitalWrite(GreenLed_PIN,HIGH);
        digitalWrite(RedLed_PIN, LOW);
      }   
      else{
        digitalWrite(GreenLed_PIN,LOW);
        digitalWrite(RedLed_PIN, HIGH);
      }
    }
    else{
      // in 12h and 24h mode green led is on all the time, red led is on during the night
      digitalWrite(GreenLed_PIN, HIGH);
      if (current_time.hour < 9 or current_time.hour > 21)
        digitalWrite(RedLed_PIN, HIGH);
      else
        digitalWrite(RedLed_PIN, LOW);
    }
  }

  if (mode == 0){
    digitalWrite(Motor_PIN,HIGH);
    digitalWrite(Heater_PIN,HIGH);
    if (lights_on){
      //scale current color to match the brightness level
      int rel_r = (current_color.r / 5.0) * brightness;
      int rel_g = (current_color.g / 5.0) * brightness;
      int rel_b = (current_color.b / 5.0) * brightness;
      int rel_w = (current_color.w / 5.0) * brightness;

      analogWrite(R_PIN, rel_r);
      analogWrite(G_PIN, rel_g);
      analogWrite(B_PIN, rel_b);
      analogWrite(W_PIN, rel_w);
    }
    else{
      analogWrite(R_PIN,0);
      analogWrite(G_PIN,0);
      analogWrite(B_PIN,0);
      analogWrite(W_PIN,0);
    } 

  digitalWrite(Heater_PIN,lights_on);
  digitalWrite(Motor_PIN, lights_on); 
  }

  if (mode == 1){
    // 12h mode
    if (current_time.hour < 9 or current_time.hour > 20){
      current_color = {0,0,0,0};
      
      heater_on = false;
      motor_on = false;
    }
    else{
      heater_on = true;
      motor_on = true;
      switch (current_time.hour){
        case 9:
        case 20:
          current_color = {255,127,0,0};
          break;
        
        case 10:
        case 19:
          current_color = {255,191,63,0};
          break;

        case 11:
        case 18:
          current_color = {255,255,127,63};
          break;

        case 12:
        case 17:
          current_color = {255,255,191,127};
          break;
        
        case 13:
        case 16:
          current_color = {255,255,255,191};
          break;

        case 14:
        case 15:
          current_color = {255,255,255,255};
          break;
      } 
    } 
    analogWrite(R_PIN,current_color.r);
    analogWrite(G_PIN,current_color.g);
    analogWrite(B_PIN,current_color.b);
    analogWrite(W_PIN,current_color.w); 

    digitalWrite(Heater_PIN,heater_on);
    digitalWrite(Motor_PIN, motor_on);  
  }

  if (mode == 2){
    // 24h mode
    if (current_time.hour < 9 or current_time.hour > 20){
      heater_on = false;
      motor_on = false;
    }
    else{
      heater_on = true;
      motor_on = true;
    }
    switch (current_time.hour){   
      case 1:
      case 2:
      case 3:
      case 4:
        current_color = {16,0,31,0};
        break;

      case 5:
      case 0:
        current_color = {31,0,63,0};
        break;

      case 6:
      case 23:
        current_color = {63,0,127,0};
        break;

      case 7:
      case 22:
        current_color = {127,31,63,0};
        break;

      case 8:
      case 21:
        current_color = {191,63,63,0};
        break;

      case 9:
      case 20:
        current_color = {255,127,0,0};
        break;
      
      case 10:
      case 19:
        current_color = {255,191,63,0};
        break;

      case 11:
      case 18:
        current_color = {255,255,127,63};
        break;

      case 12:
      case 17:
        current_color = {255,255,191,127};
        break;
      
      case 13:
      case 16:
        current_color = {255,255,255,191};
        break;

      case 14:
      case 15:
        current_color = {255,255,255,255};
        break;
    } 
    analogWrite(R_PIN,current_color.r);
    analogWrite(G_PIN,current_color.g);
    analogWrite(B_PIN,current_color.b);
    analogWrite(W_PIN,current_color.w);

    digitalWrite(Heater_PIN,heater_on);
    digitalWrite(Motor_PIN, motor_on); 
    
  }
  delay(100); 
}
