/*********************************************************************
  ESPn
  extrasensory perception via a network
  
  utilizing an arduino pro, a 128x64 OLED SPI display,
  an electric imp (and electric imp breakout board) and some accessories
 
  
  Revision Log
  Date       |Author     |Description
  -----------|-----------|-------------------------------------------
  04/05/2013  N. McBean   Initial Release based on other code
                            this code will interact with the serial
                            terminal. Used as a test board to send 
                            and receive signals from the other board
  04/07/2013 N. McBean    Will receive messages from the imp and 
                            print them in the serial console. Will
                            send messages to the imp when typed.
******************************************************************/

/*** library includes ***/
// OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*** pin definitions ***/
#define OLED_DC     11
#define OLED_CS     12
#define OLED_CLK    10
#define OLED_MOSI   9
#define OLED_RESET  13
#define SPEAKER_PIN 6
#define TRIGGER_PIN 2

/*** global creations ***/
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

String inputString = "";         // a string to hold incoming data 21 chars per line, 8 lines on size1
String oled_string = "";
boolean stringComplete = false;  // whether the string is complete
unsigned long last_screen_update;
unsigned long last_communication_timestamp;
unsigned long last_communication_age;
String console_input = "";
boolean consoleComplete = false;
int health;

volatile int trigger_button_pushed = 0;
unsigned long last_trigger;


/*** initialization and setup ***/
void setup()   {                
  /**********************************************************************************/
  /* setup display */
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Powering up...");
  display.display();

  /**********************************************************************************/  
  /* setup serial */
  Serial1.begin(19200);
  Serial.begin(9600);
  //while (!Serial) ;
  Serial.println("Welcome!");
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  oled_string.reserve(200);
  console_input.reserve(200);
  
  last_screen_update = 0;
  health = 100; 

  /**********************************************************************************/
  /* send a dummy message to the imp */
  Serial1.println("Auto comm from slave imp...,");
  
  /**********************************************************************************/
  /* setup trigger button */
  trigger_button_pushed = 0;
  pinMode(TRIGGER_PIN, OUTPUT);
  // this needs to be 0 because the first (0th) interrupt is on pin 2
  // THE STUPID LEONARDO MESSED UP NUMBERING- pin 3 = int 0
  // use PIN3 on the leonardo. use PIN2 on the pro ugh.
  attachInterrupt(0,trigger_push, RISING);
  last_trigger =0;  
}

// we hit the trigger! don't do serial operations in here. get out ASAP
// (from the documentation "Inside the attached function, delay() won't work and the 
// value returned by millis() will not increment. Serial data received while in the 
// function may be lost. You should declare as volatile any variables that you modify 
// within the attached function.

void trigger_push () {
  trigger_button_pushed = 1; 
}

/*** never ending main loop ***/
void loop() {
  
  // read from the console
  if(Serial.available() > 0)
  {
    char inChar = (char)Serial.read();

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '~') {
      consoleComplete = true;
    } else {
      // add it to the inputString:
      console_input += inChar;
    }
  }
  
  // if finished reading from teh console, send it to the imp (and print to console)
  if (consoleComplete) {
    // print to the console
    Serial.print("tx to imp: ");
    Serial.println(console_input);
    
    // send to the imp
    Serial1.println(console_input); 
   
    consoleComplete = false;
    console_input = "";
  }
  
  // read fromt he imp
  if(Serial1.available() > 0)
  {
    // get the new byte:
    char inChar = (char)Serial1.read();

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == ',') {
      stringComplete = true;
    } else {
      // add it to the inputString:
      inputString += inChar;
    }
    
  }

  // if finished reading from the imp, send it to the console
  // print the string when a newline arrives:
  if (stringComplete) {
    health = 100;
    oled_string = inputString;
    updateScreen();
    
    // maintenance for the next loop
    Serial.print("rx from imp: ");
    Serial.println(inputString); 
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    
    beep();
  }
  
  // update the screen every second
  if(millis() > last_screen_update + 1000) {
    if (health >= 5) {
      health -= 5; 
    }
    updateScreen();
  }
  
  // handle the button presses here
  if( trigger_button_pushed && millis() > last_trigger + 1000) {
    trigger_button_pushed = 0;
    Serial.println("Sending trigger!");
    // temporarily keep using the comma delimiter
    // soon will just send the one char
    Serial1.print("s,");
    last_trigger = millis();
  } else if ( trigger_button_pushed ) {
    // oh no! we bounced!
    // kill the spare
    trigger_button_pushed = 0; 
  }
}

/* Helper function, beeps 3 times */
void beep() {
  int delayms = 50;
  
  analogWrite(SPEAKER_PIN, 20);      // Almost any value can be used except 0 and 255
  delay(delayms);                    // wait for a delayms ms
  analogWrite(SPEAKER_PIN, 0);       // 0 turns it off
  delay(delayms);                    // wait for a delayms ms   
  
  analogWrite(SPEAKER_PIN, 20);      // Almost any value can be used except 0 and 255
  delay(delayms);                    // wait for a delayms ms
  analogWrite(SPEAKER_PIN, 0);       // 0 turns it off
  delay(delayms);                    // wait for a delayms ms 
  
  analogWrite(SPEAKER_PIN, 20);      // Almost any value can be used except 0 and 255
  delay(delayms);                    // wait for a delayms ms
  analogWrite(SPEAKER_PIN, 0);       // 0 turns it off
  delay(delayms);                    // wait for a delayms ms   
}

void updateScreen() {
  // display it
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(oled_string);
  
  display.drawRect(5,50,118,10,WHITE);
  
  float width = 1.18*health;
  display.fillRect(5,50,ceil(width),10,WHITE);
  
  display.display();
  last_screen_update = millis();
}
