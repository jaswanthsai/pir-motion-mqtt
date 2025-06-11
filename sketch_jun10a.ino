#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "<YOUR_SSID>";
const char* password = "<YOUR_PASSWORD>";
const char* mqtt_server = "<MQTT_SERVER_IP>";

// MQTT Broker (your Raspberry Pi IP)
const char* mqtt_server = "xx.xxx.xx.xxx";

// PIR motion sensor pin
const int pirPin = D1; // Adjust as per wiring

WiFiClient espClient;
PubSubClient client(espClient);

// Motion detection variables
bool motionDetected = false;
unsigned long motionStoppedTime = 0;
const unsigned long motionStopDelay = 5000; // 5 seconds delay to confirm motion stopped

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✅ WiFi connected");
  Serial.print("📡 IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("🔄 Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Connected");
    } else {
      Serial.print("❌ Failed, rc=");
      Serial.print(client.state());
      Serial.println(" | retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int pirState = digitalRead(pirPin);
  Serial.print("PIR raw state: ");
  Serial.println(pirState);

  if (pirState == HIGH) {
    if (!motionDetected) {
      motionDetected = true;
      Serial.println("🚨 Motion STARTED!");
      client.publish("motion", "Motion STARTED");
    }
    motionStoppedTime = 0;
  } else {
    if (motionDetected) {
      if (motionStoppedTime == 0) {
        motionStoppedTime = millis();
      } else if (millis() - motionStoppedTime > motionStopDelay) {
        motionDetected = false;
        Serial.println("✅ Motion STOPPED!");
        client.publish("motion", "Motion STOPPED");
        motionStoppedTime = 0;
      }
    }
  }
  delay(1000); // 1 second delay for easier reading
}
