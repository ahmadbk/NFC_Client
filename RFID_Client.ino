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
#include "String.h"
#include "MFRC522.h"	//include the RFID reader library
#include "WiFiUdp.h"

#define SS_PIN 2    //slave select pin
#define RST_PIN 4  //reset pin
MFRC522 mfrc522(SS_PIN, RST_PIN);        // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;//create a MIFARE_Key struct named 'key', which will hold the card information


//Definitions
#define BAUD 115200

//Network Details
//String NetworkName = "MWEB_348B11";
//String NetworkPassword = "7765B600A3";

//String NetworkName = "iPhone";
//String NetworkPassword = "ahmadkhalid";

//String NetworkName = "TP-LINK_AAE9";
//String NetworkPassword = "imranparuk";

String NetworkName = "Home WiFi";
String NetworkPassword = "0828292775";

//Client Server Details
//String Host = "192.168.88.37";
//int Port = 6950;
String Host = "";
int Port = 0;

WiFiClient client;

//Setup Loop
void setup()
{
	Serial.begin(BAUD);
	Serial.println("Starting Setup");

	pinMode(16,OUTPUT);
	digitalWrite(16, LOW);


	SPI.begin();

	mfrc522.PCD_Init();

	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}

	Connect_to_WiFi();
	delay(1000);
	receive_server_addr();
	//Connect_to_WiFi();
	Serial.println("Done Setup");

}

//Main Loop
void loop()
{	
	// Look for new cards, and select one if present
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		delay(50);
		return;
	}

	Connect_as_Client();
	Serial.print(F("Card UID:"));
	int tag = 0;
	tag = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Send_New_Data_to_Server(tag);
	byte *data;
	int dataLen = 0;
	data = receive_Data_From_Server(dataLen);
	String resp;
	for (int i = 0; i < 2; i++)
	{
		resp += (char)data[i];
	}

	Serial.println();
	Serial.println(resp);
	if (resp.equals("11"))
	{
		digitalWrite(16, HIGH);
		delay(3000);
		digitalWrite(16, LOW);
	}
	delay(1000);
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
			Serial.print("Network not available!!! Unable to Connect!");
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
void Send_New_Data_to_Server(int tagNum)
{

	Serial.println("Frame of Data to Send: ");
	String ReaderID = "1";
	String tag = String(tagNum);
	Serial.print(ReaderID+"|"+tag+"#");

	Serial.println("");
	client.println(ReaderID + "|" + tag + "#");

}

// Helper routine to dump a byte array as hex values to Serial
int dump_byte_array(byte *buffer, byte bufferSize) {
	int value = 0;
	//for (byte i = 0; i < bufferSize; i++) {
		//value += ((buffer[0] & 0xFF) | (buffer[1] & 0xFF) | (buffer[2] & 0xFF) | (buffer[3] & 0xFF)) ;
		//value1 += ((buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0]));
		//Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		//Serial.print(buffer[i], HEX);
	//}
	value += (buffer[0]) * 4;
	Serial.print(value);
	Serial.println("");

	return value;
}

//-Pointer to all bytes received is returned
byte* receive_Data_From_Server(int &no_of_bytes_received)
{
	//int Received_New_Data = 0;		//No New Data Yet
	int size = 0;
	byte *ptr_Bytes_of_Data_In = NULL;

	while (client.available())			//While there is New Data Retreive it
	{
		size++;
		ptr_Bytes_of_Data_In = (byte*)realloc(ptr_Bytes_of_Data_In, size * sizeof(byte)); //grow array

		byte data_byte = client.read();					//read a byte
		ptr_Bytes_of_Data_In[size - 1] = data_byte;		//assign to byte array
		no_of_bytes_received = size;					//New Data Has been Retreived
	}
	if (no_of_bytes_received) {
		Serial.println("");
		Serial.print("No. of Bytes Received: ");
		Serial.println(no_of_bytes_received);
	}

	return ptr_Bytes_of_Data_In;
} //END receive_Data_From_Server

int receive_server_addr()
{
	WiFiUDP Udp;
	unsigned int localUdpPort = 23;
	char incomingPacket[255];

	Udp.begin(localUdpPort);
	Serial.println("LISTENING.......");
	bool conn = true;

	while (conn)
	{
		int packetSize = Udp.parsePacket();
		if (packetSize)
		{
			Serial.println((Udp.remoteIP().toString()) + " " + Udp.remotePort());
			int len = Udp.read(incomingPacket, 255);
			Serial.println(incomingPacket);
			Host = Udp.remoteIP().toString();
			Port = atoi(incomingPacket);
			conn = false;
		}
	}
	Udp.stop();
} 
