/*
 WiFi-Kit8 water depth monitor Arduino program.

 The Kit8 ESP8266 board reads the volage off an outboard
 ADS1015 I2C ADC.  The ADC voltage is the output of a Milone 
 Technologies eTape liquid level sensor.  The program calculates
 liquid height from the voltage read, displays height on OLED
 display, and publishes new value approximately every 20 seconds.

 Much of this code comes from the ESP8266 MQTT example
 at https://github.com/knolleary/pubsubclient

 Other code snippets come from examples in the Arduino IDE.

 Paul Zawada
 3 May 2020
*/


#include <Adafruit_ADS1015.h>

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "heltec.h" // alias for `#include "SSD1306Wire.h"`
#define ETAPE_LEN 15  /* eTape size = full scale = adc input = 5V */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* Network config */

const char* ssid = "Your-SSID-Here";
const char* password = "Your-WiFi-Password";
const char* mqtt_server = "host.example.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char mqtt_msg[50];
int adc0_prev = 0;

int time_count = 0;

Adafruit_ADS1015 ads;

/* Calculate the adc slope (inches per count) */
float adc_slope = ETAPE_LEN / 1666.7; /* Full scale 1667 */


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {

  Serial.begin(9600);
  Serial.println("Getting single-ended readings from AIN0..");

  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();
  
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);

  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Heltec.display->clear();
      
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0,0, String("Connecting WiFi..."));
  Heltec.display->display();
      
  setup_wifi();

  Heltec.display->clear();
  
  client.setServer(mqtt_server, 1883);

}

void loop() {
  // put your main code here, to run repeatedly:

  int16_t adc0;
  float adc0_v, eTapeDepth;
  String adc0_vs, eTapeDepth_s;
 

if (!client.connected()) {
    reconnect();
  }
  client.loop();


  /* Read adc port 0 */

  adc0 = ads.readADC_SingleEnded(0);
  
  /* Calculate voltage for debug purposes */
  
  adc0_v = adc0 *.003;
  adc0_vs = String(adc0_v,2);


 /* Calculate fluid depth measure by eTape = {adc count} * {adc slope} */
  

  eTapeDepth = adc_slope * adc0;
  eTapeDepth_s = String(eTapeDepth,2);

  Serial.println(String("adc0: " + String(adc0) +" = "+ adc0_vs+" V"));

  Serial.println(String("eTape Depth = "+ eTapeDepth_s + " in."));
 
  
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0,0, String("Depth: " + eTapeDepth_s + " in."));
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0,20,String("adc0: " + String(adc0) +" = "+ adc0_vs+" V"));
 
    // write the buffer to the display
  Heltec.display->display();

/* 
   We are going to publish MQTT after 20 interations of this loop (~1 sec 
   per iteration) or earlier if the adc count has changed more than 20 counts
   since we last published.
*/

  if (time_count >= 20 | abs(adc0-adc0_prev)> 20 ) {
      snprintf (mqtt_msg, 50, "{\"depth_inches\" : %.3f }", eTapeDepth);

      Serial.print("Publishing: ");
      Serial.println(mqtt_msg);

      Heltec.display->clear();
      Heltec.display->setFont(ArialMT_Plain_16);
      Heltec.display->drawString(0,0, String("Depth: " + eTapeDepth_s + " in."));
      Heltec.display->setFont(ArialMT_Plain_10);
      Heltec.display->drawString(0,20,String("Publishing MQTT"));
      Heltec.display->display();
      client.publish("home/sump", mqtt_msg);
      adc0_prev = adc0;
      time_count = 0;
  } else time_count++;
    
  delay(1000);
  Heltec.display->clear();

}
