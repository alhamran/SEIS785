#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information  (account info are defined in a tab that attched this skecth)

WiFiClient client(255);

// The number of times to trigger the action if the condition is met (in case somthing goes wrong, you will not recive more unstopable txts or messages, it happed during testing)
int maxCalls = 5;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

int inputPin = A0;

int streamInterval = 30000; // streaming interval in milliseconds
uint32_t lastStreamRunTime = millis() - streamInterval; // store the time of the last stream write

//Distance
const int trigPin = 2;
const int echoPin = 6;

// defines variables
long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600);
  
  
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);

  int wifiStatus = WL_IDLE_STATUS;

  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("FAIL");

    // If there's no WiFi shield, stop here
    while(true);
  }

  Serial.println("OK");

  // Try to connect to the local WiFi network
  while(wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID);

    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
    delay(5000);
  }

  
  // Initialize pins
  //pinMode(inputPin, INPUT);

  Serial.println("Setup complete.\n");
}

void loop() {
  //int sensorValue = digitalRead(inputPin);
  //Serial.println("Sensor: " + String(sensorValue));

  // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    
    // Calculating the distance (in CM)
    distance= duration*0.034/2;
    
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);

  if (distance < 15) {
    if (calls < maxCalls) {
      Serial.println("\nTriggered! Calling SendEmail Choreo...");
      runSendEmail(distance);
      calls++;
    } else {
      Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
    }
  }

  uint32_t now = millis();
  if (now - lastStreamRunTime >= streamInterval) {
    lastStreamRunTime = now;
    stream();
  }
}

void stream() {
  TembooChoreo stream(client);

  // Invoke the Temboo client
  stream.begin();

  // Set Temboo account credentials
  stream.setAccountName(TEMBOO_ACCOUNT);
  stream.setAppKeyName(TEMBOO_APP_KEY_NAME);
  stream.setAppKey(TEMBOO_APP_KEY);

  // Identify the Choreo to run
  stream.setChoreo("/Library/Util/StreamSensorData");

  // Set the Streaming profile to use
  stream.setProfile("myStream");

  // Generate sensor data to stream
  String pinData = "{";
  pinData += "\"Fisrt\":" + String(digitalRead(2));
  pinData += ",";
  pinData += "\"Second\":" + String(digitalRead(6));
  pinData += "}";

  // Add sensor data as an input to the streaming Choreo
  stream.addInput("SensorData", pinData);
  // NOTE: for debugging set "Async" to false (indicating that a response should be returned)
   stream.addInput("Async", "false");

  // Stream the data; when results are available, print them to serial
  stream.run();

  while(stream.available()) {
    char c = stream.read();
    Serial.print(c);
  }
  stream.close();
}

void runSendEmail(int sensorValue) {
  TembooChoreo SendEmailChoreo(client);

  // Set Temboo account credentials
  SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  String UsernameValue = "arduinoust@gmail.com";
  SendEmailChoreo.addInput("Username", UsernameValue);
  String ToAddressValue = "ahamran9@gmail.com";
  SendEmailChoreo.addInput("ToAddress", ToAddressValue);
  String SubjectValue = "test";
  SendEmailChoreo.addInput("Subject", SubjectValue);
  String MessageBodyValue = "test";
  SendEmailChoreo.addInput("MessageBody", MessageBodyValue);
  String PasswordValue = "kxvanybolillqkvu";
  SendEmailChoreo.addInput("Password", PasswordValue);

  // Identify the Choreo to run
  SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");

  // Run the Choreo
  unsigned int returnCode = SendEmailChoreo.run();

  // Read and print the error message
  while (SendEmailChoreo.available()) {
    char c = SendEmailChoreo.read();
    Serial.print(c);
  }
  Serial.println();
  SendEmailChoreo.close();
}
