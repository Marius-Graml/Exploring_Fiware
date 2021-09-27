#include <WiFiClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>
#include <Arduino_JSON.h>
#include "DHT.h"

#define DHT11PIN 16

DHT dht(DHT11PIN, DHT11);
float humi = 0;
float temperat = 0.0;

void connectToNetwork()
{
  const char *ssid = "***";                   // Choose name of WiFi network you want to connect to
  const char *password = "***"; // Insert WiFi password

  Serial.println('\n');

  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.println("\n"); // Send the IP address of the ESP8266 to the computer
}

void sendData()
{
  String servername = "http://<IP_Address>:1026/v2/entities/urn:ngsi-ld:Station:001/attrs";
  Serial.println("Sending data...\n");
  String httpPostData = "{\"temperature\":{\"type\":\"Number\", \"value\":"+String(temperat)+"},\"humidity\":{\"type\":\"Number\", \"value\":"+String(humi)+"}}";
  HTTPClient client;

  client.begin(servername);
  client.addHeader("Content-Type", "application/json");
  int httpResponseCode = 0;
  int postrequest_tries = 0;
  while (httpResponseCode != 200 && postrequest_tries < 10)
  {
    httpResponseCode = client.POST(httpPostData);
    postrequest_tries++;
  }
  Serial.print("HTTP status code: " + String(httpResponseCode) + "\n");
  if(httpResponseCode == 415)
  {
    Serial.println("Data was sent!\n");
  }
  client.end();
}

void getData()
{
  String servername = "http://<IP_Address>:1026/v2/entities/urn:ngsi-ld:Station:001?options=values&attrs=temperature,humidity";
  Serial.println("Quering data...\n");
  HTTPClient client;

  client.begin(servername);
  int tries = 0;
  int httpResponseCode = 0;
  while (httpResponseCode != 200 && tries < 10)
  {
    httpResponseCode = client.GET(); // Send HTTP GET request and receive status code
    tries++;
  }
  Serial.print("HTTP status code: " + String(httpResponseCode) + "\n");

  if (httpResponseCode == 200)
  {
    String payload = client.getString(); //get payload of the url
    Serial.println("Received data from Context Broker:");
    Serial.println(payload);
  }
  else
  {
    Serial.println("Error on HTTP request: " + String(httpResponseCode));
  }
  client.end();
}

void measurement()
{
  dht.begin();

  humi = dht.readHumidity();
  temperat = dht.readTemperature();
  Serial.println("Measured data:");
  Serial.print("Temperature: ");
  Serial.print(temperat);
  Serial.print("ºC ");
  Serial.print("Humidity: ");
  Serial.println(humi);
  Serial.println("\n");
}

void setup()
{
  Serial.begin(9600);
  connectToNetwork();
}

void loop()
{
  measurement();
  delay(1000);
  sendData();
  delay(1000);
  getData();
  delay(5000);
  Serial.println("\n\n\n");
}