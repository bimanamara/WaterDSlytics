#include "BluetoothSerial.h"
#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif

//Wifi
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "iPhone"
#define WIFI_PASSWORD "bimasmansa"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAYA6IXorCKPvEk8MW8LusEfTEMCORieiI"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://kedalamanair-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "kedalamanair@gmail.com"
#define USER_PASSWORD "112233"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String presPath = "/pressure";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

float temperatur;
float humidity;
float pressure;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;

BluetoothSerial SerialBT;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation
float Threshold = 0;
bool water_flag;
bool msg_flag;
char message;

float Vstep = 0.345;  //kecepatan stepper
unsigned long currentTime;
float L;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESPDESPRO"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  msg_flag = false;
  if (SerialBT.available()){
    message = SerialBT.read();
    unsigned long previousTime = millis();
    
    while(message == 'G'){
      tdsValue = analogRead(TdsSensorPin);
      Serial.println(TdsSensorPin);

      currentTime = millis();
      
      if (tdsValue > Threshold){
        water_flag = true;
        L = (Vstep * (currentTime - previousTime))/1000;
        Serial.println(L);
      SerialBT.flush();  
      SerialBT.disconnect();
      SerialBT.end();
      delay(5000);//delay step run + 5 detik
      delay(10000);//10 detik persiapan wifi
      initWiFi();
      configTime(0, 0, ntpServer);
      
      // Assign the api key (required)
      config.api_key = API_KEY;
        
      // Assign the user sign in credentials
      auth.user.email = USER_EMAIL;
      auth.user.password = USER_PASSWORD;
        
      // Assign the RTDB URL (required)
      config.database_url = DATABASE_URL;
        
      Firebase.reconnectWiFi(true);
      fbdo.setResponseSize(4096);
        
      // Assign the callback function for the long running token generation task */
      config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
        
      // Assign the maximum retry of token generation
      config.max_token_generation_retry = 5;
        
      // Initialize the library with the Firebase authen and config
      Firebase.begin(&config, &auth);
        
      // Getting the user UID might take a few seconds
      Serial.println("Getting User UID");
      while ((auth.token.uid) == "") {
        Serial.print('.');
        delay(1000);
      }
      // Print user UID
      uid = auth.token.uid.c_str();
      Serial.print("User UID: ");
      Serial.println(uid);
        
      // Update database path
      databasePath = "/UsersData/" + uid + "/readings";
      break;
      }else {
        water_flag = false;
      }

      if (water_flag){
        message = '\0';
      }
      delay(20);
    }
  }
  
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    temperatur = 0.01 + random(0,100);
    pressure = 0.01 + random(0,100);
    humidity++;
    
    parentPath= databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(L));
    json.set(humPath.c_str(), String(humidity));
    json.set(presPath.c_str(), String(pressure));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
