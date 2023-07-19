#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define RED_PIN 5
#define GREEN_PIN 4
#define BLUE_PIN 0

#define WIFI_SSID "Mika"
#define WIFI_PASSWORD "Stinkie100"
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "home/room/golira"
#define MQTT_STATUS_TOPIC "home/room/golira/status"

WiFiClient espClient;
PubSubClient client(espClient);

int mode = 0;  // 0 for solid colors, 1 for breathing, 2 for rainbow
int speed = 5;  // speed of transitions, can be adjusted

int currentRed = 255;
int currentGreen = 255;
int currentBlue = 255;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  analogWriteRange(255);  // to map analogWrite between 0-255

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  clear();  // Clear colors

  if (messageTemp.startsWith("RGB")) {
    // Split the message by comma
    int commaIndex1 = messageTemp.indexOf(",");
    int commaIndex2 = messageTemp.lastIndexOf(",");

    // Get each value
    currentRed = messageTemp.substring(3, commaIndex1).toInt();
    currentGreen = messageTemp.substring(commaIndex1 + 1, commaIndex2).toInt();
    currentBlue = messageTemp.substring(commaIndex2 + 1).toInt();

    // Update the LEDs
    analogWrite(RED_PIN, currentRed);
    analogWrite(GREEN_PIN, currentGreen);
    analogWrite(BLUE_PIN, currentBlue);

    mode = 0;
  } else if (messageTemp == "BREATHING") {
    mode = 1;
    Serial.println("Breathing Mode Activated!");
  } else if (messageTemp == "RAINBOW") {
    mode = 2;
    Serial.println("Rainbow Mode Activated!");
  } else if (messageTemp == "SPEEDUP") {
    speed -= 1;
    if (speed < 1) speed = 1;
    Serial.println("Speed Increased!");
  } else if (messageTemp == "SPEEDDOWN") {
    speed += 1;
    if (speed > 10) speed = 10;
    Serial.println("Speed Decreased!");
  } else if (messageTemp == "STATUS") {
    String statusMessage = "Mode: ";
    if (mode == 0) {
      statusMessage += "Color, ";
    } else if (mode == 1) {
      statusMessage += "Breathing, ";
    } else if (mode == 2) {
      statusMessage += "Rainbow, ";
    }
    statusMessage += "Speed: " + String(speed);
    statusMessage += ", Color: RGB(" + String(currentRed) + "," + String(currentGreen) + "," + String(currentBlue) + ")";
    client.publish(MQTT_STATUS_TOPIC, statusMessage.c_str());
  }
}

void clear() {
  analogWrite(RED_PIN, 255);  // invert output for common anode
  analogWrite(GREEN_PIN, 255);  // invert output for common anode
  analogWrite(BLUE_PIN, 255);  // invert output for common anode
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (mode == 1) {
    // Breathing mode
    for (int i = 0; i < 255; i++) {
      analogWrite(RED_PIN, constrain(currentRed + i, 0, 255));
      analogWrite(GREEN_PIN, constrain(currentGreen + i, 0, 255));
      analogWrite(BLUE_PIN, constrain(currentBlue + i, 0, 255));
      delay(speed * 10);
    }
    for (int i = 255; i > 0; i--) {
      analogWrite(RED_PIN, constrain(currentRed + i, 0, 255));
      analogWrite(GREEN_PIN, constrain(currentGreen + i, 0, 255));
      analogWrite(BLUE_PIN, constrain(currentBlue + i, 0, 255));
      delay(speed * 10);
    }
  } else if (mode == 2) {
    // Rainbow mode
    for (int i = 0; i < 255; i++) {
      analogWrite(RED_PIN, i);
      delay(speed * 10);
    }
    for (int i = 0; i < 255; i++) {
      analogWrite(GREEN_PIN, i);
      delay(speed * 10);
    }
    for (int i = 0; i < 255; i++) {
      analogWrite(BLUE_PIN, i);
      delay(speed * 10);
    }
    clear();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("Golira")) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
