/*********************************************************************
  ESPn
  extrasensory perception via a network
  
  utilizing an arduino pro, a 128x64 OLED SPI display,
  an electric imp (and electric imp breakout board) and some accessories
 
  This is the IMP code
  
  Based off of "Transmit data between UART and Input/OutputPorts on 
  the impee" by: Jim Lindblom SparkFun Electronics license: BeerWare
  
  PIN1- switch 4 (doesn't work)
  PIN2- wifi status LED (output), also connected to arduino input
  PIN5- Imp Transmit (connect to Rx of arduino)
  PIN7- Imp Receive  (connect to Tx of arduino)
  PIN8- switch 2 (works great)  
  PIN9- pad (nc)
  
  
  Revision Log
  Date       |Author     |Description
  -----------|-----------|-------------------------------------------
  09/26/2012  J. Lindblom Public code release from sparkfun
  04/03/2013  N. McBean   Added watchdog function to prevent imp from
                          sleeping (was sleeping after 15m)
  04/07/2013  N. McBean   Changed to use callback instead of polling
  04/29/2013  N. McBean   Testing other IO for switches.
  05/22/2013  N. McBean   Cleanup
******************************************************************/

// impeeIn will override the InputPort class. 
// Whenever data is received to the impee, we'll jump into the set(c) function defined within
class impeeIn extends InputPort
{
    name = "UART Out";
    type = "string";
    
    // This function takes whatever character was sent to the impee
    // and sends it out over the UART5/7. We'll also toggle the txLed
    function set(c)
    {
        hardware.uart57.write(c);
        //server.log(format("Internet to Ard %s", c)); // send the character out to the server log. Optional, great for debugging
        server.show(c);
    }
}

local impeeInput = impeeIn();  // assign impeeIn class to the impeeInput
local impeeOutput = OutputPort("UART In", "string");  // set impeeOutput as a string
local sw2 = OutputPort("SW2","integer");

// configure the UART to call the readserial() function when it gets data
// callbacks are much better than polling!
// i couldn't get the slower uarts to quickly work. but this seems fine, so meh.
function initUart()
{
    hardware.configure(UART_57);    // Using UART on pins 5 and 7
    hardware.uart57.configure(19200, 8, PARITY_NONE, 1, NO_CTSRTS,readserial); // 19200 baud worked well, no parity, 1 stop bit, 8 data bits
}

// This function is called whenever pin8//sw2 changes state.
function switched() {
    local local_sw2 = hardware.pin1.read(); // Read the pin state
    
    sw2.set(local_sw2);           // Send it to the server
    server.show(local_sw2?"on":"off");      // Update the user-visible label
    server.log(local_sw2?"on":"off");
}

function initPins()
{
    // LED for wifi status
    hardware.pin2.configure(DIGITAL_OUT_OD_PULLUP);
    hardware.pin2.write(1);  // 1 = off, 0 = on
}

// If there is data in the UART57 buffer, this function will be called and we will
// this will read as much of it as it can, and send it out of the impee's outputPort.
function readserial()
{
    local byte = hardware.uart57.read();    // read the UART buffer
    while (byte != -1)  // otherwise, we keep reading until there is no data to be read.
    {
        server.log(format("Ard to Internet %c", byte)); // send the character out to the server log. Optional, great for debugging
        impeeOutput.set(byte);  // send the valid character out the impee's outputPort
        byte = hardware.uart57.read();  // read from the UART buffer again (not sure if it's a valid character yet)
    }
    
}

// sometimes the imp likes to go to sleep. this stops that from happening
// i probably don't need this since i have so much other code running, but i've grown attached to it by now
function watchdog() {
  imp.wakeup(5*60, watchdog);
  server.log("watchdog");
}

// lets keep checking our wireless health. 
// check rapidly when not connected. back off when connected
function checkWifi() {
  
  // according to documentation, above -67dbm is good enough
  // http://devwiki.electricimp.com/doku.php?id=electricimpapi:imp:rssi
  if( imp.rssi() > -67) {
    // wifi is good
    hardware.pin2.write(0); // 0 = on
    // don't bother checking for another minute almost
    imp.wakeup(58,checkWifi);
    //server.log("Wifi good");
  } else {
    // wifi is bad
    hardware.pin2.write(1); // 1 = off
    // check in a few seconds
    imp.wakeup(5,checkWifi);
    //server.log("Wifi bad");
  }
    
}

// This is where our program actually starts! Previous stuff was all function and variable declaration.
// This'll configure our impee. It's name is "UartCrossAir", and it has both an input and output to be connected:
imp.configure("UART_MASTER", [impeeInput], [impeeOutput, sw2]);

hardware.pin1.configure(DIGITAL_IN, switched);
switched();

initUart(); // Initialize the UART, called just once
initPins();
watchdog();
checkWifi();

// The end