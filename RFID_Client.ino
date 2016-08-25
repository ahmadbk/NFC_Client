/*
Name:	RFID_Client
Created: 8/16/2016	05:55:13 PM
Author:	Ahmad
*/

#include "Arduino.h"
#include "WiFiClientSecure.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
#include "ESP8266WiFiType.h"
#include "ESP8266WiFiSTA.h"
#include "ESP8266WiFiScan.h"
#include "ESP8266WiFiMulti.h"
#include "ESP8266WiFiGeneric.h"
#include "ESP8266WiFiAP.h"
#include "ESP8266WiFi.h"
#include <SPI\SPI.h>		//include the SPI bus library
#include "MFRC522.h"	//include the RFID reader library

#define SS_PIN 2    //slave select pin
#define RST_PIN 4  //reset pin
MFRC522 mfrc522(SS_PIN, RST_PIN);        // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;//create a MIFARE_Key struct named 'key', which will hold the card information


//Definitions
#define BAUD 115200

//Network Details
String NetworkName = "MWEB_348B11";
String NetworkPassword = "7765B600A3";

//Client Server Details
String Host = "192.168.1.86";
int Port = 7123;

WiFiClient client;

//Setup Loop
void setup()
{
	//pinMode(SS_PIN, OUTPUT);
	//pinMode(RST_PIN, OUTPUT);

	Serial.begin(BAUD);

	SPI.begin();

	mfrc522.PCD_Init();

	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}

	Connect_to_WiFi();
	Connect_as_Client();
	Send_New_Data_to_Server();
	Serial.println("Done Setup");

}

//Main Loop
void loop()
{	
	// Look for new cards, and select one if present
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		delay(50);
		Serial.println("Not Working");
		return;
	}
	Serial.println("Working");

	Serial.print(F("Card UID:"));
	dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Serial.println();
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
	Serial.println("WiFi Connected!!!");
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

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
	for (byte i = 0; i < bufferSize; i++) {
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}
}