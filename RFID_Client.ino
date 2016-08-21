/*
Name:	RFID_Client
Created: 8/16/2016	05:55:13 PM
Author:	Ahmad
*/

#include "WiFiClientSecure.h"
#include "WiFiClient.h"
#include "ESP8266WiFiType.h"
#include "ESP8266WiFiSTA.h"
#include "ESP8266WiFiScan.h"
#include "ESP8266WiFiMulti.h"
#include "ESP8266WiFiGeneric.h"
#include "ESP8266WiFiAP.h"
#include "ESP8266WiFi.h"

//Definitions
#define BAUD 115200

//Network Details
String NetworkName = "MWEB_348B11";
String NetworkPassword = "7765B600A3";

//Client Server Details
String Host = "192.168.1.75";
int Port = 6950;

WiFiClient client;

//Setup Loop
void setup()
{
	Serial.begin(BAUD);
	Connect_to_WiFi();
	Connect_as_Client();
	Send_New_Data_to_Server();
}

//Main Loop
void loop()
{

}

//Attempt Connection to WiFi --> Attemps 20 times
void Connect_to_WiFi(void)
{
	int attempt_count = 20;
	bool network_available = false;

	WiFi.disconnect();
	Serial.println("Attempting Connection to: ");
	Serial.print(NetworkName);

	//Check If Desired Network is available, Return if unavailable
	int n = WiFi.scanNetworks();
	if (n == 0)
	{
		Serial.println("No Networks Found. Cannot Connect!");
		return;
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			if (WiFi.SSID(i) == NetworkName)
				network_available = true;
			delay(10);
		}
		if (!network_available)
		{
			Serial.print("Network: ");
			Serial.print(NetworkName);
			Serial.print(" not available. Unable to Connect!");
			return;
		}
	}

	//If Network Available, Attempt Connection, Return if failed
	WiFi.begin(NetworkName.c_str(), NetworkPassword.c_str());
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		attempt_count--;
		if (!attempt_count)
		{
			Serial.println("Password Incorrect! WiFi Connection Failed!");
			return;
		}
	}
	Serial.println("WiFi Connected!!");
	Serial.print("IP Address is:");
	Serial.print(WiFi.localIP());
	Serial.println("");
	return;
}

//Attempt Connection to Server
void Connect_as_Client(void)
{
	Serial.println("Attempting Connection to Server...");
	if (client.connect(Host.c_str(), Port))
	{
		Serial.println("Connected to Server!");
	}
	else
		Serial.println("Connection to Server Not Possible.");
}

//Send Data to Server
void Send_New_Data_to_Server(void)
{

	Serial.println("Frame of Data to Send: ");
	String data = "Hello from the other side!";
	Serial.print(data);

	Serial.println("");
	client.println(data);

}