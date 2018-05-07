#include "bv4612_I.h"
#include <TimeLib.h> 
#include <Timezone.h>
#include "config.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>


WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

// Create an instance of the server
// specify the port to listen on as an argument
// WiFiServer server(80);

Timezone UKT(BST, GMT);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get TZ abbrev
unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP; // time.nist.gov NTP server address

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
time_t prevDisplay = 0; // when the digital clock was displayed

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

int timeout;

// 128x64 display with 16 way keypad

// On ESP8266, I2C pins are D1 = SCL, D2 = SDA

// 7 bit adddress is used
BV4612 ui(0x35);


int state=0;
const int state_norm=0;
const int state_prefs=1;
const int state_set=2;
const int state_status=3;
int last_state=-1;
int wificonnected=0;
const int uidelay=20;

void setup()
{
   Serial.begin(115200);
   // set up bv4612 display
   ui.reset();
   delay(uidelay);
   ui.contrast(contrast);
   ui.bl(bl);
   ui.font(1);  // 1= 5x8,  2= 8x8 bold, 3 = double height
   ui.clear();
   ui.print("Connecting to ");
   ui.setCursor(0,1);
   ui.print(ssid);
  
   // Connect to WiFi network
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

   // set a timeout for wifi connection - will retry in main loop.
//   int timeout=0;
//   while ((WiFi.status() != WL_CONNECTED) && timeout>0 ) {
//      delay(500);
//      Serial.print(".");
//      timeout--;
//   }

   if (WiFi.status() == WL_CONNECTED)
   {
      wifiReconnect();
   } else {
      Serial.println();
      Serial.println("WiFi not connected - timeout");
   }
   ui.clrBuf();

  // MQTT initialisation.
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

};

void wifiReconnect() {
   Serial.println("");
   Serial.println("WiFi connected");
   ui.setCursor(0,0);
   ui.print("WiFi OK");
  
   // Print the IP address
   Serial.println(WiFi.localIP());  
   Serial.println("Starting UDP");
   udp.begin(localPort);
   Serial.print("Local port: ");
   Serial.println(udp.localPort());
   Serial.println("waiting for sync");
   setSyncProvider(getNtpTime);
  
   //get a random server from the pool
   WiFi.hostByName(ntpServerName, timeServerIP); 
   wificonnected=1;
}


void loop()
{
  checkKeys();
  if ( state != last_state) {
    ui.clear();
    last_state=state;
  }
  switch (state) {
    case state_norm:
      displayNorm();
      break;
    case state_prefs:
      displayPrefs();
      break;
    case state_set:
      displaySet();
      break;
    case state_status:
      displayStatus();
      break;
  }
  delay(200);  
  if (WiFi.status() == WL_CONNECTED)
  { 
    if (wificonnected==0) {
      wifiReconnect();
    }
    if (!client.connected()) {
      reconnect();
    } 
    client.loop();
  } else {
    wificonnected=0;
  }
}

void displayNorm() {
  if (WiFi.status() != WL_CONNECTED)
  {
      ui.setCursor(0,0);
      ui.print("No WiFi");
      delay(uidelay);
  } else if (!client.connected())
  {
      ui.setCursor(0,0);
      ui.print("No mqtt");
      delay(uidelay);    
  } else {
      ui.setCursor(0,0);
      ui.print("OK     ");
      delay(uidelay);        
  }
  showClock();
  delay(uidelay);
  showTemps(0); 
  showControls();
}

void displayPrefs() {
  ui.setCursor(0,0);
  ui.print("Preferences");
  ui.setCursor(0,4);
  ui.print("Cont:");
  ui.print(contrast);
  ui.print(" ");
}

void displaySet() {
  ui.setCursor(0,0);
  ui.print("Setup Page");
  
}

void displayStatus() {
  ui.setCursor(0,0);
  ui.print("Status Page");
  ui.setCursor(0,1);
  ui.print(ssid);
  ui.setCursor(0,2);
  ui.print(WiFi.localIP());
  delay(uidelay);
  for (int i=0; i<rfsensors; i++) 
  {
    ui.setCursor(0,3+i*2);
    ui.print(rfloc[i]);
    ui.print(" @ ");
    ui.setCursor(0,4+i*2);
    ui.print(day(rftime[i]));
    ui.print("/");
    ui.print(month(rftime[i]));
    ui.print(" ");
    uip2d(hour(rftime[i]));
    ui.print(":");
    uip2d(minute(rftime[i]));
    ui.print(":");
    uip2d(second(rftime[i]));
    delay(uidelay);
  }
}

void uip2d(int n) {
  if (n<10) {
    ui.print("0");
  }
  ui.print(n);
  delay(uidelay);
}


void sendMqttMessage(String topic, String data) {
   String topicc="cmnd/";
   topicc.concat(topic);
   char topicbuf[40], databuf[4];

   topicc.toCharArray(topicbuf,40);
   data.toCharArray(databuf,4);
   client.publish(topicbuf,databuf);
}



void checkKeys() {
  char k, buf[30];
  if(ui.keysBuf()) {
    k = ui.key();
    sprintf(buf,"Key=%d  ",k);
    if (state==state_status) {
      ui.setCursor(0,3);
      ui.print(buf);  
    }
    Serial.println(buf);

    switch (k+state*20) {
       case 1:
       case 2:
       case 3:
       case 4:
         actuator[k-1]=1-actuator[k-1];
         sendMqttMessage(mqttName[k-1],actuator[k-1]?"ON":"OFF");
         break;
       case 1+state_prefs*20:
          contrast++;
          ui.contrast(contrast);
          break;
       case 5+state_prefs*20:
          contrast--;
          ui.contrast(contrast);
          break;
       case 4+state_prefs*20:
          bl=1-bl;
          ui.bl(bl);
          break;
       case 13:
       case 13+20:
       case 13+40:
       case 13+60:
          state=state_norm;
          break;
       case 14:
       case 14+20:
       case 14+40:
       case 14+60:
          state=state_prefs;
          break;
       case 15:
       case 15+20:
       case 15+40:
       case 15+60:
          state=state_set;
          break;
       case 16:
       case 16+20:
       case 16+40:
       case 16+60:
          state=state_status;
          break;
     
       case 12:
          client.publish(outTopic, "Test message");
          Serial.println("Sending MQTT message");
          break;
    }
  }

}

void showTemps(int num) {
  int xloc=0;  // 60 for over to the right.
  int yloc=1;
  ui.font(1);
  for (int i=0; i<sensors; i++)
  {
    ui.setCursor(xloc,yloc+i);
    ui.print(locs[i]);
    ui.print(" ");
    ui.setCursor(xloc+21,yloc+i);
    ui.print(temps[i]/10);
    ui.setCursor(xloc+32,yloc+i);
    ui.print(".");
    ui.setCursor(xloc+37,yloc+i);
    ui.print(temps[i]%10);
    ui.print(" ");
    ui.setCursor(xloc+47,yloc+i);
    ui.print(humids[i]/10);
    ui.print("% ");
  }
  delay(uidelay);
}

void showControls() {
  for (int i=0; i<actuators; i++) {
    ui.setCursor(80,i);
    ui.print(actName[i]);
    ui.setCursor(106,i);
    ui.print(actuator[i]?"ON ":"Off");
  }
}


void showClock() 
{
  int clockx=62;
  ui.font(2);
  ui.setCursor(0,6);
  ui.print(days[weekday()]);
  ui.print("  ");
  ui.setCursor(0,7);
  ui.print(day());
  ui.print(" ");
  ui.print(months[month()]);
  ui.print(" ");
  delay(uidelay);

  ui.font(3);
  ui.setCursor(clockx,6);
  if (hour() <10){
    ui.print("0");
  }
  ui.print(hour());
  ui.setCursor(clockx+17,6);
  ui.print(":");
  ui.setCursor(clockx+23,6);
  if (minute() <10){
    ui.print("0");
  }
  ui.print(minute());
  ui.setCursor(clockx+40,6);
  ui.print(":");  
  ui.setCursor(clockx+46,6);
  if (second() <10){
    ui.print("0");
  }
  ui.print(second());
  delay(uidelay);

  ui.font(1);
  delay(uidelay);
}


/*-------- MQTT code ---------*/


void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;
  String stopic = (char*)topic;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((stopic.indexOf("tele/sensor") == 0) || (stopic.indexOf("tele/garage") == 0)) {
    int num=parseTopic(topic);
    temps[num]=parsePayload(message,1);
    humids[num]=parsePayload(message,2);
//    Serial.print(num);
//    Serial.print(" ");
//    Serial.print(locs[num]);
//    Serial.print(" ");
//    Serial.print(temps[num]);
//    Serial.print(" ");
//    Serial.println(humids[num]);
  } else if (stopic.indexOf("stat") ==0) {
    parseRecStat(stopic,message);  
  } else if (stopic.indexOf("tele/rf_bridge") ==0) {
    parseDoorOpen(topic,message);
  }
}

void parseDoorOpen(String topic, String payload)
{
  int loc=payload.indexOf("Data\":\"");
  String id=payload.substring(loc+7,loc+13);
  Serial.println(id);
  for (int i=0; i<rfsensors; i++) 
  {
    if (id.equals(rfid[i])) {
      rftime[i]=now();
      Serial.print(rfloc[i]);
      Serial.print(" opened at ");
      Serial.println(rftime[i]);
      break;
    }
  }
}



void parseRecStat(String topic, String payload) 
{
  for (int i=0; i<actuators; i++) {
    if (topic.indexOf(mqttName[i]) >0) {
      Serial.print(mqttName[i]);
      Serial.print(" setting to ");
      Serial.println(payload);
      if (payload.equals("ON")) {
        actuator[i]=1;
      } else {
        actuator[i]=0;
      }

      break;
    }
  }
}

int parseTopic(String topic) 
{

  // TODO replace this with a for loop over the mqttSens array returning the index of the one that matches.
  //garage is a special case and is allocated 0;
  
  String search = "sensor";
  int index = topic.indexOf(search)+search.length();
  int num;
  if (index<search.length()) {
    num=0;
  } else {
    num=(topic.charAt(index)-'0');
  }
  return num;
}


int parsePayload(String payload, int select)
{
  String search;

  if (select==1) {
    search = "Temperature\":";
  } else if (select==2) {
    search = "Humidity\":";
  }
  int index = payload.indexOf(search)+search.length();
  int temp=((payload.charAt(index)-'0')*100);
  temp=temp+((payload.charAt(index+1)-'0')*10);
  temp=temp+((payload.charAt(index+3)-'0'));
  return temp;
}



void reconnect() {
  // Loop until we're reconnected
//  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Heating Controller booted");
      // ... and resubscribe
      client.subscribe(inTopic);
      client.subscribe("tele/sensor4/SENSOR");
      client.subscribe("tele/rf_bridge/RESULT");
      for (int i=0; i<sensors; i++) {
         char topicbuf[40];
         mqttSens[i].toCharArray(topicbuf,40);
         client.subscribe(topicbuf);
      }
      for (int i=0; i<actuators; i++) {
         String topicc="stat/";
         topicc.concat(mqttName[i]);
         char topicbuf[40];
         topicc.toCharArray(topicbuf,40);
         client.subscribe(topicbuf);
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for(int i = 0; i<50; i++){
        delay(1);
      }
    }
//  }
}


/*-------- NTP code ----------*/

time_t getNtpTime()
{
   time_t utc, local;
   while (udp.parsePacket() > 0) ; // discard any previously received packets
   //get a random server from the pool
   WiFi.hostByName(ntpServerName, timeServerIP); 
   Serial.println("Transmit NTP Request");
   sendNTPpacket(timeServerIP);
   uint32_t beginWait = millis();
   while (millis() - beginWait < 1500) {
      int size = udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
         Serial.println("Receive NTP Response");
         udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
         unsigned long secsSince1900;
         // convert four bytes starting at location 40 to a long integer
         secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
         secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
         secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
         secsSince1900 |= (unsigned long)packetBuffer[43];
         utc = secsSince1900 - 2208988800UL;
         local = UKT.toLocal(utc);
         return local;
      }
   }
   Serial.println("No NTP Response :-(");
   return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see URL above for details on the packets)
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
   packetBuffer[1] = 0;     // Stratum, or type of clock
   packetBuffer[2] = 6;     // Polling Interval
   packetBuffer[3] = 0xEC;  // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12]  = 49;
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;
   // all NTP fields have been given values, now
   // you can send a packet requesting a timestamp:                 
   udp.beginPacket(address, 123); //NTP requests are to port 123
   udp.write(packetBuffer, NTP_PACKET_SIZE);
   udp.endPacket();
}

