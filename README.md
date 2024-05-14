# Esp32-mqtt


# Requirements
ESP-32 > Any ESP-32 board with Wifi

PubSubClient from https://pubsubclient.knolleary.net/

DGT sensor library by Adafruit from https://github.com/adafruit/DHT-sensor-library

# Wiring to the Host Board

![wiring](https://github.com/machbase/esp32-mqtt/assets/31678089/bbb5aba2-8805-4759-9eca-8d334a3b9caf)

# Install Machbase neo
Please follow instructions from https://docs.machbase.com/neo/getting-started/

You should create table after install Machbase neo TSDB with following SQL

```sql
CREATE TAG TABLE tag (name VARCHAR(20) PRIMARY KEY, time DATETIME BASETIME, value DOUBLE SUMMARIZED) WITH ROLLUP (hour)
```

# Things to be changed before build

- Wifi (ssid, password)
```c
// WiFi
const char *ssid = "MACHBASE_AP"; // Enter your Wi-Fi name
const char *password = "xxxxxxxx";  // Enter Wi-Fi password
WiFiClient wifi_connection;
```
- Timzone related settings
```c
//Time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*9; // TZ Diff in seconds
const int daylightOffset_sec = 0;  
```
- Machbase Internal MQTT Broker IP/Port
```c
// MQTT
const char *mqtt_server = "192.168.1.131"; // Your Machbase neo server IP
const char *topic = "db/append/TAG"; // default
const int   mqtt_port = 5653; // default
```
