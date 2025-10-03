/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete instructions at https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
*********/
#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <TinyGPS++.h>


// Network and Firebase credentials
#define WIFI_SSID "#"
#define WIFI_PASSWORD "#"

#define Web_API_KEY "#"
#define DATABASE_URL "#"
#define USER_EMAIL "#"
#define USER_PASS "#"

#define RXD2 16
#define TXD2 17

#define GPS_BAUD 9600

// User function
void processData(AsyncResult &aResult);

TinyGPSPlus gps;

HardwareSerial gpsSerial(2);




// Authentication
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

// Timer variables for sending data every 10 seconds
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // 10 seconds in milliseconds

// Variables to send to the database
int intValue = 0;
float floatValue = 0.01;
String stringValue = "";

void setup(){
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Wifi connected");

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");
  
  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);
  
  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

void loop(){
  // Maintain authentication and async tasks
  app.loop();

  if (app.ready()){ 
    unsigned long start = millis();

    while (millis() - start < 1000) {
      while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
      }

      if (gps.location.isUpdated()) {
        Serial.print("LAT: ");

        Database.set<float>(aClient, "/cow12/lat", gps.location.lat(), processData, "RTDB_Send_Float");
        Database.set<float>(aClient, "/cow12/log", gps.location.lng(), processData, "RTDB_Send_Float");
        Database.set<float>(aClient, "/cow12/sp", gps.location.kmph(), processData, "RTDB_Send_Float");
      }
    }
  }
}

void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
}
