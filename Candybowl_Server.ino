#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed

#define WLAN_SSID       "gundam"           // cannot be longer than 32 characters!
#define WLAN_PASS       "JoyceLee0101"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           23    // What TCP port to listen on for connections.

// Create an instance of the CC3000 server listening on the specified port.
Adafruit_CC3000_Server candyServer(LISTEN_PORT);

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

void setup(void)
{
  // Set up the serial port connection.
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 
  
  // Set up the CC3000, connect to the access point, and get an IP address.
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  Serial.println(F("Connected!"));
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  
  while (!displayConnectionDetails()) {
    delay(1000);
  }

  // Start the candy bowl server.
  candyServer.begin();
  
  //Start BMP sensor
  if(!bmp.begin())
  {
    /* Problem detecting BMP */
    Serial.print("No BMP detected!");
    while(1);
  }
  
  Serial.println(F("Listening for connections..."));
}

void loop(void)
{
  /* New sensor event */
  sensors_event_t event;
  bmp.getEvent(&event);
  
  /* variables */
  float temperature;
  
  // Handle a connected client.
  Adafruit_CC3000_ClientRef client = candyServer.available();
  if (client) {
     // Check if there is data available to read.
     if (client.available() > 0) {
       uint8_t ch = client.read();
       // Respond to candy a bowl status query.
       if (ch == '?') {
         client.fastrprint("Temperature: ");
         if (event.pressure) {
           client.println(temperature);
         }
         else {
           client.println("No Temperature");
         }
       }
     }
  }
}

// Display connection details like the IP address.
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

