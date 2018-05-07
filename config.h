

const char* ssid = "shipfi";
const char* password = "mw51v1tm";
const char* mqtt_server = "openhabianpi";
const char* clientID = "HeatingControl";
const char* outTopic = "tele/heatcont/STATUS";
const char* inTopic = "tele/heatcont/COMMAND";
const char* ntpServerName = "0.uk.pool.ntp.org";

// Change this for local conditions, leave variable names as BST for summer and GMT for winter.
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        //British Summer Time -  "UTC + 1" or GMT + 1
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         //British Winter Time -  "UTC + 0" or GMT 


int contrast=10;
int bl=1;
String days[8] = {"","Sun","Mon","Tues","Weds","Thurs","Fri","Sat"};
String months[13] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

// temperatures as read from sensors in degrees *10;
const int sensors=4;
int temps[sensors];
int humids[sensors];
String locs[sensors]={"Gar","Lng","Lnd","Kit"};      
String mqttSens[sensors]={"tele/garage/SENSOR","tele/sensor1/SENSOR","tele/sensor2/SENSOR","tele/sensor3/SENSOR"};


const int actuators=4;
int actuator[actuators];
String actName[actuators]={"Heat","Watr","Pump","UFlr"};
String mqttName[actuators]={"heating-1/POWER1","heating-1/POWER2","heating-1/POWER3","pro4/POWER1"};

const int rfsensors=2;
String rfid[rfsensors]={"F27EE1","C09B41"};
String rfloc[rfsensors]={"Front Door","Back door"};
time_t rftime[rfsensors];

