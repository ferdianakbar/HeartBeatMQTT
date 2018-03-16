#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <SPI.h>
#include <Wire.h>

// The Ticker/flipper routine
Ticker flipper;

//  VARIABLES
int blinkPin = 15;                // pin to blink led at each beat
int fadePin = 12;                 // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.


// Update these with values suitable for your network.

const char* ssid = "duitali";
const char* password = "passwerdapa?";
const char* mqtt_server = "192.168.43.211";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[20];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  interruptSetup();
  
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      interruptSetup();
      // Once connected, publish an announcement...
      client.publish("/tubes/pulse", "Device connected");
      // ... and resubscribe
     // client.subscribe("inTopic");
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(6000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    interruptStop();
    reconnect();
    
 }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    String test = String(BPM);
    char msgTest[5];
    test.toCharArray(msgTest,test.length()+1);
    Serial.print("Publish message: ");
    Serial.println(msgTest);
    client.publish("tubes/pulse", msgTest);
  }
  readBeat(BPM);
}


void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
 }

 void readBeat(int BPM){
    Serial.print("\nBPM = ");
    String a = String(BPM);
    Serial.println(a);
    Serial.print("IBI = ");
    Serial.println(IBI);
    Serial.print("Signal  ");
    Serial.println(Signal); 
    delay(999); 
 }

