/*This sketch was written for Arduino Leonardo and connectivity is with Adafruit CC3000 */
/* -- Kevin Monk April 6, 2015 */


#include <PubSubClient.h>

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

#define aref_voltage 3.3


#define WLAN_SSID       "*********"
#define WLAN_PASS       "*********"

#define WLAN_SECURITY   WLAN_SEC_WPA2


#define ADAFRUIT_CC3000_IRQ   3 

#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); 
                                         

//analog pins                                         
int tempPin = 0;
int luxPin = 1;


//cc3000 and PubSub variables                                         
void callback(char* topic, byte* payload, unsigned int length) {
    
}

uint32_t ip;

Adafruit_CC3000_Client client = Adafruit_CC3000_Client();
PubSubClient mqttClient("146.148.90.97", 1883, callback, client);


void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  
  
  //The following is for the wifi connection
  
  Serial.println(F("\nInitializing..."));
  
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.println(F("\nDeleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  
 
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
  //Now the broker connection 
  
  if (!cc3000.getHostByName("146.148.90.97", &ip)) {
    Serial.println(F("Couldn't resolve!"));
  }
  
  Serial.println("Connecting to MQTT Broker....");
 
  if (mqttClient.connect("ArduinoLeoClient-CC3000-A4")) {
    Serial.println("Connected");
  } else {
    Serial.println("Test1");
  }
}

void loop() {
  
  int tReading = analogRead(tempPin);


  // converting reading to voltage
  float voltage = tReading * 5.0;
  voltage /= 1024.0; 
  
  Serial.println();
  
  float temperatureC = (voltage - 0.5) * 100 ; 
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  Serial.print(temperatureF); Serial.println(" degrees F");
  
  Serial.print(analogRead(luxPin));
  float luxRead = analogRead(luxPin);
  
  Serial.println();
  
  
  //Conversion from float is necessary for .publish
  char temp[10];
  dtostrf(temperatureF, 0, 2, temp);
  
  char lux[10];
  dtostrf(luxRead, 0, 0, lux);
  
  
  mqttClient.publish("kmk/fake/lux", lux);
  mqttClient.publish("kmk/fake/temperature", temp);
 

  delay(5000);  
}


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

