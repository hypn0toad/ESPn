/*********************************************************************
  M. A. I. D. O. (L)
  mechanically assisted inventive device offering love
  
  Written by Nate McBean for Christine Chan!
  
  Revision Log
  Date       |Author     |Description
  -----------|-----------|-------------------------------------------
  03/31/2013  N. McBean   Initial Release- screen and piezo buzzer
  04/01/2013  N. McBean   Takes UART from the computer and displays
                            it on the OLED Display
*********************************************************************/

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

/*** global creations ***/
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

/*** initialization and setup ***/
void setup()   {                
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  
  // initialize serial:
  //Serial1.begin(9600);
  Serial.begin(9600);
  //while (!Serial) ;
  Serial.println("Welcome!");
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  // init done
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Ello");
  display.display();
}

/*** never ending main loop ***/
void loop() {
  //beep();
  
  if(Serial.available() > 0)
  {
    // get the new byte:
    char inChar = (char)Serial.read();

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '|') {
      stringComplete = true;
    } else {
      // add it to the inputString:
      inputString += inChar;
    }
    
  }

  // print the string when a newline arrives:
  if (stringComplete) {
    // display it
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(inputString);
    display.display();
    
    // maintenance for the next loop
    Serial.println(inputString); 
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    
    
  }
  
  /*
  display.clearDisplay();
  display.invertDisplay(false);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello, world!");
  
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println(3.141592);
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("0x"); display.println(0xDEADBEEF, HEX);
  
  display.display();
  delay(2000);
  
  display.invertDisplay(true);
  delay(2000);
  
  display.clearDisplay();
  display.invertDisplay(false);  
  display.display();  
  delay (2000);
  */
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

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}


