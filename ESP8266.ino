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
bool colorChange = false;

int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

int rainbowStep = 0;

struct RGB {
  int r;
  int g;
  int b;
};

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

  if (messageTemp.startsWith("RGB")) {
    Serial.println(messageTemp);
    mode = 0;
    Serial.println("Switching to solid color mode");
    // Split the message by comma
    int commaIndex1 = messageTemp.indexOf(",");
    int commaIndex2 = messageTemp.lastIndexOf(",");

    Serial.println("commaIndex1: " + String(commaIndex1));
    Serial.println("commaIndex2: " + String(commaIndex2));

    // Get each value
    currentRed = messageTemp.substring(3, commaIndex1).toInt();
    currentGreen = messageTemp.substring(commaIndex1 + 1, commaIndex2).toInt();
    currentBlue = messageTemp.substring(commaIndex2 + 1).toInt();

    colorChange = true;

    // Update the LEDs
    writeRGB(RED_PIN, GREEN_PIN, BLUE_PIN, currentRed, currentGreen, currentBlue);

    Serial.println("Received RGB values:");
    Serial.println(currentRed);
    Serial.println(currentGreen);
    Serial.println(currentBlue);

  } 
  else if (messageTemp == "BREATHING") {
    mode = 1;
    Serial.println("Switching to breathing mode");
    Serial.println("Breathing Mode Activated!");
  } 
  else if (messageTemp == "RAINBOW") {
    mode = 2;
    Serial.println("Switching to rainbow mode");
    Serial.println("Rainbow Mode Activated!");
  } 
  else if (messageTemp == "SPEEDUP") {
    speed -= 3;
    if (speed < 1) speed = 1;
    Serial.println("Speed Increased!");
  } 
  else if (messageTemp == "SPEEDDOWN") {
    speed += 3;
    if (speed > 10) speed = 10;
    Serial.println("Speed Decreased!");
  }
  else if (messageTemp == "BREATHING_OFF") {
    mode = 0;
    analogWrite(RED_PIN, currentRed);
    analogWrite(GREEN_PIN, currentGreen);
    analogWrite(BLUE_PIN, currentBlue);
    Serial.println("Breathing Mode Deactivated!");
  }
  else if (messageTemp == "RAINBOW_OFF") {
  mode = 0;
  analogWrite(RED_PIN, currentRed);
  analogWrite(GREEN_PIN, currentGreen);
  analogWrite(BLUE_PIN, currentBlue);
  Serial.println("Rainbow Mode Deactivated!");
}
  else if (messageTemp == "STATUS") {
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

RGB hslToRgb(float h, float s, float l) {
  float r, g, b;

  if(s == 0){
    r = g = b = l; // achromatic
  } else {
    auto hue2rgb = [](float p, float q, float t) {
      if(t < 0) t += 1;
      if(t > 1) t -= 1;
      if(t < 1/6.0) return p + (q - p) * 6.0f * t;
      if(t < 1/2.0) return q;
      if(t < 2/3.0) return p + (q - p) * (2/3.0f - t) * 6.0f;
      return p;
    };

    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    r = hue2rgb(p, q, h + 1/3.0);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1/3.0);
  }

  RGB color = {int(r * 255), int(g * 255), int(b * 255)};
  
  // Debugging
  //Serial.println("Converted HSL(" + String(h) + ", " + String(s) + ", " + String(l) + ")");
  //Serial.println("To RGB(" + String(color.r) + ", " + String(color.g) + ", " + String(color.b) + ")");

  return color;
}

void writeRGB(int redPin, int greenPin, int bluePin, int r, int g, int b) {
  analogWrite(redPin, 255 - r);
  analogWrite(greenPin, 255 - g);
  analogWrite(bluePin, 255 - b);
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
      if (colorChange) {
        colorChange = false;
        break;
      }
      writeRGB(RED_PIN, GREEN_PIN, BLUE_PIN, currentRed * i / 255, currentGreen * i / 255, currentBlue * i / 255);
      delay(speed);
      if (mode != 1) return;
    }
    for (int i = 255; i > 0; i--) {
      if (colorChange) {
        colorChange = false;
        break;
      }
      writeRGB(RED_PIN, GREEN_PIN, BLUE_PIN, currentRed * i / 255, currentGreen * i / 255, currentBlue * i / 255);
      delay(speed);
      if (mode != 1) return;
    }
  } else if (mode == 2) {
    // Rainbow mode
    //Serial.println("Running rainbow animation");
    if(rainbowStep >= 360){
      rainbowStep = 0;
    }

    RGB color = hslToRgb(rainbowStep / 360.0, 1.0, 0.5);
    writeRGB(RED_PIN, GREEN_PIN, BLUE_PIN, color.r, color.g, color.b);
    
    // Debugging
    Serial.println("Writing RGB(" + String(color.r) + ", " + String(color.g) + ", " + String(color.b) + ")");
    
    rainbowStep++;
    delay(speed);
  } 
  delay(speed);
  if (mode != 2) return;
 else {
    //Serial.println("Displaying solid color");
    writeRGB(RED_PIN, GREEN_PIN, BLUE_PIN, currentRed, currentGreen, currentBlue);
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
