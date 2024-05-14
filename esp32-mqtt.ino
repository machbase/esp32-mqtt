#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>

#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT11
//DHTTYPE = DHT11, but there are also DHT22 and 21
DHT dht(DHTPIN, DHTTYPE); // constructor to declare our sensor

// WiFi
const char *ssid = "MACHBASE_AP"; // Enter your Wi-Fi name
const char *password = "xxxxxx";  // Enter Wi-Fi password
WiFiClient wifi_connection;

//Time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*9;
const int daylightOffset_sec = 0;

// MQTT
const char *mqtt_server = "192.168.1.131";
const char *topic = "db/append/TAG";
const int   mqtt_port = 5653;
PubSubClient mqtt_client(wifi_connection);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(200)
char msg[MSG_BUFFER_SIZE];

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

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
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-";
    clientId += WiFi.localIP();
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println("connected");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(callback);
}

int64_t get_timestamp_milis() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000LL;
//	return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

void loop() {
  int64_t current_time;
  char * id;
  String clientId = "ESP32-";

  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
 
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    clientId += WiFi.localIP();
    id = (char*)clientId.c_str();
   
    current_time = get_timestamp_milis() * 1000000LL;

    // The DHT11 returns at most one measurement every 1s
    float h = dht.readHumidity();
    //Read the moisture content in %.
    float t = dht.readTemperature();
    //Read the temperature in degrees Celsius

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed reception");
      return;
      //Returns an error if the ESP32 does not receive any measurements
    }

    // Transmits the measurements received in the serial monitor
    snprintf (msg, MSG_BUFFER_SIZE, "[ [\"%s.hum\",%lld,%f], [\"%s.tem\",%lld,%f] ]", 
        id, current_time, h, id, current_time, t);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqtt_client.publish(topic, msg);
    printLocalTime();
  }
}
