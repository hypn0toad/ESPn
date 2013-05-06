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
  05/05/2013  N. McBean   Changed communication protocol, now 
                            displays on screen when receive [], otherwise
                            handle each character individually.
                            Initial communication works with button press
                            TODO: debounce? needed?
******************************************************************/

/*** library includes ***/
// OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*** debug switches and settings ***/
String revID = "ver2013-05-05-A";
unsigned int comm_frequency     = 21; // pings the server every how many sec?
int screen_update_freq = 1;

unsigned int max_age_rx_comm = 30;

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

unsigned long last_communication_age;
int health;

// internal stuff
boolean flash_led_status;
boolean debugio;
boolean current_identity; // 0 = c, 1 = n
boolean receiving_message;
boolean other_person_online;
unsigned long last_communication_timestamp;
unsigned long last_ping_tx_timestamp, last_ping_rx_timestamp;
unsigned long this_second;

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
  this_second = 0;
  
  // setup communication
  last_communication_timestamp = 0;
  last_ping_tx_timestamp = 0;
  last_ping_rx_timestamp = 0;
  other_person_online = false;
  
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
  
  // initialize this to a value, it doesn't matter what it is
  flash_led_status  = true;
  // initial value is that we're not in the middle of a message.
  receiving_message = false;
  
  // read some startup values
  debugio = digitalRead(SW7_PIN);
  current_identity = digitalRead(SW6_PIN);
  
  // init done
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40,8);
  display.println("ESPn");
  
  display.setTextSize(1);
  display.setCursor(20,25);

  if ( current_identity ) {
    display.println("  Hello Nate!");     
  } else {
    display.println("Hello Christine!");     
  }

  display.setCursor(20,55);
  display.println(revID);
  display.display();
  
  delay(5000);
}

/*** never ending main loop ***/
// this does all of the main processing, but is specially crafted so that it should hopefully take 
// as close to zero time as possible. sure serial communication will eat up some time, but most events
// are essentially scheduled (checking timestamps), there should not be any delays in this code
void loop() {
  this_second = floor(millis()/1000);

  // first see if we've received any serial communication (this would be from the IMP).
  // this input will be decoded to see if it is part of a message, or just a command
  if(Serial.available() > 0)
  {
    // get the new byte (we receive one byte/character at a time)
    char inChar = (char)Serial.read();
    
    switch (inChar) {
      // check for starting character (this allows plaintext to be displayed)
      case '[':
        receiving_message = true;
        break;
      // check for ending character
      case  ']':
        stringComplete    = true;
        receiving_message = false;
        break;
      // any other character- advanced handle
      default:
        // if in the middle of a message, save the character
        if (receiving_message) { 
          inputString += inChar;
        // otherwise, decode the COMMAND character
        } else {
          switch (inChar) {
            // t = trigger = someone pressed the button on the other device
            case 't':
               health = 100;
               beep();
               break; 
            // s = status = automatic ping from the other device
            case 's':
               last_ping_rx_timestamp = this_second;
               break;
          }
        } 
    }
  }

  // check if we should ping the server
  if ((this_second > last_ping_tx_timestamp + comm_frequency) | (last_ping_tx_timestamp == 0)) {
    Serial.print("s"); 
    last_ping_tx_timestamp = this_second;
  }
  
  // check for button press
  if (digitalRead(TX_BUT_PIN)) {
    last_communication_timestamp = this_second;
    beep();
    Serial.print("t"); 
  }  
  
  // if we just received a message, handle it.
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
  
  // update the OLED display once a second (meh doesn't need to be faster)
  if(this_second > last_screen_update + screen_update_freq) {
    if (health >= 5) {
      health -= 5; 
    }
    updateScreen();
  }

  // update the OTHER PERSON PRESENT indicator
  if ( (this_second > last_ping_rx_timestamp + max_age_rx_comm) || (last_ping_rx_timestamp == 0)) { 
    other_person_online = false;
  } else {
    other_person_online = true;
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

// /* Helper function, build an age based on a timestamp */
// String timetoage(unsigned long stamp) {
//   unsigned long h, m, s;
//   unsigned long diff;
//   String age = "";

//   // lets just store the difference in seconds because who really cares about MS
//   diff = floor((millis() - stamp)/1000);

//   h = floor(diff / (60*60));
//   if( h > 0) {
//     diff -= (h * 60 * 60);
//     age += h;
//     age += "h";
//   }

//   m = floor(diff / 60);
//   if (m > 0) {
//     diff -= (m * 60);
//     age += m;
//     age += "m";
//   }

//   s = diff;
//   age += s;
//   age += "s";

//   return age;
// }

void updateScreen() {
  // first check wifi
  int wifi_not_connected = digitalRead(WIFI_STATUS_PIN);
  
  if( debugio ) {
    // Blink some LEDs
    digitalWrite(LED1_PIN, flash_led_status);
    digitalWrite(LED2_PIN, flash_led_status);
    digitalWrite(ACK_LED_PIN, flash_led_status);
    flash_led_status = !flash_led_status;
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Elapsed: ");
    display.println(this_second);
    
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
  } else {
    // display it
    display.clearDisplay();
    display.setTextSize(1);
    

    // first line = wifi status
    display.setCursor(115,0);
    if(wifi_not_connected) {
      display.print(":(");
    } else if (other_person_online) {
      display.print("XD");
    } else {
      display.print(":)");
    }

    display.setCursor(0,9);
    if(other_person_online) {
      display.print("See ");
    } else {
      display.print("Can't find ");
    }
    display.println(current_identity?"chanchan!":"maido!");


    // second line = time since last comm
    display.print("Last rx ");
    display.print(this_second-last_ping_rx_timestamp);
    display.println(" ago");

    

    // 4th line message if there is one
    display.println(oled_string);
    
    display.drawRect(5,50,118,10,WHITE);
    
    float width = 1.18*health;
    display.fillRect(5,50,ceil(width),10,WHITE);
  }
    
  display.display();
  last_screen_update = this_second;

  // not really the screen, but this is called less frequently
  if (other_person_online) {
    digitalWrite(ACK_LED_PIN,1);
  } else {
    digitalWrite(ACK_LED_PIN,0);
  }
}

void error(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Fatal error");
  display.println(message);
  
  // freeze the arduino so it doesn't try to do anything else
  while (1) {}
}
