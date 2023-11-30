#include <WiFi.h>
#include "time.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h" //Provide the token generation process info.
#include "addons/RTDBHelper.h" //Provide the RTDB payload printing info and other helper functions.

#define DEVICE_UID "ESP32" // Device ID
#define DATABASE_URL "https://iot-study-8372a-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyAX5ATWg02gU5FPTg4uQbmM1s8kahEj_YQ "
#define USER_EMAIL "yxf4ts@virginia.edu"
#define USER_PASSWORD "password"

const char* ntpServer = "pool.ntp.org"; //server to fetch time
const long gmtOffset_sec = -3600*5; //offset from gmt is -5 hours
const int daylightOffset_sec = 0;

const char *ssid = "wahoo"; //Wifi Network
FirebaseData fbdo; // Firebase Realtime Database Object
FirebaseAuth auth; // Firebase Authentication Object
FirebaseConfig config; // Firebase configuration Object
String databasePath = ""; // Firebase database path
String parentPath = ""; //To be updated each time
String fuid = ""; // Firebase Unique Identifier
unsigned long elapsedMillis = 0; // Stores the elapsed time from device start up
unsigned long update_interval = 500; // The frequency of sensor updates to firebase, set to 3 seconds

String timestamp;
FirebaseJson json;

void wifi_init(){
  // Get MAC Address:
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA); // Acting as client
  WiFi.begin(ssid);
  Serial.println("\nConnecting to Wifi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void firebase_init() {
  config.api_key = API_KEY; // configure firebase API Key 
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL; // configure firebase realtime database url
  Firebase.reconnectWiFi(true); // Enable WiFi reconnection 
  Serial.println("------------------------------------");
  Serial.println("Signing in...");
  config.token_status_callback = tokenStatusCallback; // Assign the callback function for the long running token generation task, see addons/TokenHelper.h
  Firebase.begin(&config, &auth); // Initialise the firebase library

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  fuid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(fuid);
}

void database_test() {// Check that update interval has elapsed, device is authenticated and the firebase service is ready.
  struct tm timeinfo;
  time_t epoch_time; 
  time(&epoch_time); //assigns raw time value to epoch_time (seconds) for use in graphs/organization
  getLocalTime(&timeinfo); //fills a struct with time value like month, year, etc.
  int year = timeinfo.tm_year + 1900; //needed because it will return year since 1900
  int month = timeinfo.tm_mon + 1; //needed because it will start at month 0
  //timestamp is display time value that's easy to veiw
  timestamp = String(year) + "-" + String(month) + "-" + String(timeinfo.tm_mday) + " " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
  json.set("/Temp", String(random(32,100)));
  json.set("/Humidity", String(random(0,100)));
  json.set("/UV", String(random(0,11)));
  json.set("/Timestamp", epoch_time);

  parentPath = databasePath + DEVICE_UID + "/" + String(timestamp); //each data entry is under the device name > timestamped

  if (millis() - elapsedMillis > update_interval && Firebase.ready()){
    elapsedMillis = millis();
    Serial.println("------------------------------------");
    //Writes JSON object to firebase using firebase object fbdo at the specified path (c_str becaus arduino string had issues), returns ok if successful or error code if not
    Serial.printf("Writing to database... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "OK" : fbdo.errorReason().c_str());
  }
}

void setup()
{
  Serial.begin(9600);
  wifi_init();
  firebase_init();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  database_test();
}
