//
// branchController 
//
// Code for a Teensy 3.2 controller to implement a branch controller which
// controls up to 8 separate LED strips, based on instructions received
// over TCP-IP.
//
// Status: TEST CODE
// Author: Joel Spolsky


#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include <FastLED.h>
#include <SPI.h>
#include <Ethernet.h>


//
// FastLED strip configuration
//
#define NUM_LEDS_PER_STRIP 44
#define NUM_STRIPS 8

  // Pin layouts on the teensy 3 will be: 2,14,7,8,6,20,21,5

#define BRIGHTNESS 32

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

// 
// Ethernet configuration
//

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
#define ETHERNET_CS_PIN 10    /* which pin are we using to signal to the Ethernet board that we want to communicate with it */
uint8_t buffer[257];


void setup() 
{
  // Initialize LEDs
  
  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP);
  LEDS.setBrightness(BRIGHTNESS);


#ifdef ETHERNET_IS_ENABLED
  // Initialize Serial Debugger
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    // TODO GET AN ONBOARD OLED SO WE DON'T HAVE TO WAIT FOR SERIAL PORT!
  }
  Serial.println("Branch Controller PoC");
  
  // Initialize Ethernet
  
  Ethernet.init(ETHERNET_CS_PIN);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);


  // TO DO BETTER ERROR HANDLING AND STATUS MESSAGES BELOW

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) 
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP()); 
#endif
}

void loop() 
{

  static uint8_t hue = 0;
  
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
      if (j % 20 == 0) leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::White;
    }
  }

  // Set the first n leds on each strip to show which strip it is
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j <= i; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::White;
    }
  }

  hue++;

  LEDS.show();
  LEDS.delay(1);
#ifdef ETHERNET_IS_ENABLED
  handleEthernet();
#endif

}




void handleEthernet() {

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {

//    Serial.println("new client");

    size_t cbReadTotal = 0;
    while (client.connected()) {

          size_t cbAvail = 0;

          if ((cbAvail = client.available()) > 0)
          {
  //            Serial.write("cbAvail "); Serial.println(cbAvail);

            
              size_t cbToRead = cbAvail;
              if (cbToRead > 256) cbToRead = 256;
              cbReadTotal += client.read(buffer, cbToRead);
              buffer[cbToRead] = '\0';

    //          Serial.write((char*)buffer);

                if (buffer[cbToRead-1] == '}')
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html> hi you sent ");
                      
                    char rgch[12];
                    sprintf(rgch, "%d", cbReadTotal);
                    client.println(rgch);
                    
                    client.println("</html>");
                
                    // give the web browser time to receive the data
                    delay(1);
                    
                    // close the connection:
                    client.stop();
    //                Serial.println("client disconnected");
                }
          }
    }


  }
}
