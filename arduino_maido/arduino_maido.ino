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
  04/01/2013  N. McBean   Takes UART from the CLOUD (e-imp) and displays
                            it on the OLED Display O_o (matches with 
                            UART_Basic (V1) prog. pin5 imp to pin0 arduino
  04/02/2013  N. McBean   Added beep function. Added countown and 
                            rectangle bar drawing
  04/28/2013  N. McBean   Added specific pins for PWB. Tested board 
                            (all good!)
******************************************************************/

/*** library includes ***/
// OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*** debug switches ***/
#define DEBUG_IO       0

/*** pin definitions ***/
#define OLED_DC         11
#define OLED_CS         12
#define OLED_CLK        10
#define OLED_MOSI       9
#define OLED_RESET      13
#define SPEAKER_PIN     6
#define WIFI_STATUS_PIN 5

#define LED1_PIN    4
#define LED2_PIN    7
#define ACK_LED_PIN 8
#define TX_BUT_PIN  2
#define ACK_BUT_PIN 3
#define SW1_PIN     14
#define SW5_PIN     15
#define SW6_PIN     16
#define SW7_PIN     17

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
int health;
boolean tempvar;


/*** initialization and setup ***/
void setup()   {                
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  
  // initialize serial:
  Serial.begin(19200);
  //Serial.begin(9600);
  //while (!Serial) ;
  //Serial.println("Welcome!");
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  oled_string.reserve(200);
  
  last_screen_update = 0;
  health = 100;
  
  // init done
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Ello");
  display.display();
  
  // setup communication
  last_communication_timestamp = 0;
  
  // setup wifi status pin
  pinMode(WIFI_STATUS_PIN, INPUT);
  digitalWrite(WIFI_STATUS_PIN, 1);
  
  // setup other pins
  pinMode(LED1_PIN,    OUTPUT);
  pinMode(LED2_PIN,    OUTPUT);
  pinMode(ACK_LED_PIN, OUTPUT);
  pinMode(TX_BUT_PIN,  INPUT);
  pinMode(ACK_BUT_PIN, INPUT);
  pinMode(SW1_PIN,     INPUT);
  pinMode(SW5_PIN,     INPUT);
  pinMode(SW6_PIN,     INPUT);
  pinMode(SW7_PIN,     INPUT);  
  
  tempvar = true;
}

/*** never ending main loop ***/
void loop() {
  //beep();
  
//  if(millis() > last_communication_timestamp + 20000) {
//    Serial.println("AUTOCHECK!,");
//    beep();
//    last_communication_timestamp = millis();  
//  }
  
  if(Serial.available() > 0)
  {
    // get the new byte:
    char inChar = (char)Serial.read();

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == ',') {
      stringComplete = true;
    } else {
      // add it to the inputString:
      inputString += inChar;
    }
    
  }

  // print the string when a newline arrives:
  if (stringComplete) {
    health = 100;
    oled_string = inputString;
    updateScreen();
    
    // maintenance for the next loop
    //Serial.println(inputString); 
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    
    beep();
  }
  
  if(millis() > last_screen_update + 1000) {
    if (health >= 5) {
      health -= 5; 
    }
    updateScreen();
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
  // first check wifi
  int wifi_not_connected = digitalRead(WIFI_STATUS_PIN);
  
  if( DEBUG_IO ) {
    //  pinMode(LED1_PIN,    OUTPUT);
    //  pinMode(LED2_PIN,    OUTPUT);
    //  pinMode(ACK_LED_PIN, OUTPUT);
    //  pinMode(TX_BUT_PIN,  INPUT);
    //  pinMode(ACK_BUT_PIN, INPUT);
    //  pinMode(SW1_PIN,     INPUT);
    //  pinMode(SW5_PIN,     INPUT);
    //  pinMode(SW6_PIN,     INPUT);
    //  pinMode(SW7_PIN,     INPUT);  
    digitalWrite(LED1_PIN, tempvar);
    digitalWrite(LED2_PIN, tempvar);
    tempvar = !tempvar;
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Elapsed: ");
    display.println(millis());
    
    display.print("Trig Button: ");
    display.println(digitalRead(TX_BUT_PIN));
    display.print("Ack Button : ");
    display.println(digitalRead(ACK_BUT_PIN));   
    
    display.print("SW1: ");
    display.println(digitalRead(SW1_PIN));
    display.print("SW5: ");
    display.println(digitalRead(SW5_PIN));
    display.print("SW6: ");
    display.println(digitalRead(SW6_PIN));
    display.print("SW7: ");
    display.println(digitalRead(SW7_PIN));
  }  
  // if wifi isn't connected print an error
  else if (wifi_not_connected) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Not connected to WiFi");
    
  // else wifi is connected, continue!
  } else {
    // display it
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(oled_string);
    
    display.drawRect(5,50,118,10,WHITE);
    
    float width = 1.18*health;
    display.fillRect(5,50,ceil(width),10,WHITE);
  }
    
  display.display();
  last_screen_update = millis();
}
