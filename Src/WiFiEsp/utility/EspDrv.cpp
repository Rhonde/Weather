/*--------------------------------------------------------------------
 This file is part of the Arduino WiFiEsp library.

 The Arduino WiFiEsp library is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 The Arduino WiFiEsp library is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with The Arduino WiFiEsp library.  If not, see
 <http://www.gnu.org/licenses/>.
 --------------------------------------------------------------------*/

//#include <Arduino.h>
#include <pgmspace.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>
#include <ctype.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include "EspDrv.h"
#include "queue.h"
#include "serial.h"
#include "RingBuffer.h"
#include "debug.h"

#define NUMESPTAGS 5

const char* ESPTAGS[] =
{ "\r\nOK\r\n", "\r\nERROR\r\n", "\r\nFAIL\r\n", "\r\nSEND OK\r\n",
		" CONNECT\r\n" };

typedef enum
{
	TAG_OK, TAG_ERROR, TAG_FAIL, TAG_SENDOK, TAG_CONNECT
} TagsEnum;

EspDrv::EspDrv()
{
	// Array of data to cache the information related to the networks discovered
	for (int ix = 0; ix < WL_NETWORKS_LIST_MAXNUM; ix++)
	{
		sprintf(m_networkSsid[ix], "%d", ix + 1);
		m_networkRssi[ix] = 0;
		m_networkEncr[ix] = 0;
	}

	// Cached values of retrieved data
	for (int ix = 0; ix < WL_SSID_MAX_LENGTH; ix++)
		m_ssid[ix] = 0;

	for (int ix = 0; ix < WL_MAC_ADDR_LENGTH; ix++)
	{
		m_bssid[ix] = 0;
		m_mac[ix] = 0;
	}
	for (int ix = 0; ix < WL_IPV4_LENGTH; ix++)
		m_localIp[ix] = 0;

	for (uint ix = 0; ix < sizeof(m_fwVersion); ix++)
		m_fwVersion[ix] = 0;

	m_bufPos = 0;
	m_connId = 0;

	m_remotePort = 0;
	for (uint ix = 0; ix < sizeof(m_remoteIp); ix++)
		m_remoteIp[ix] = 0;

	m_ringBuf = new RingBuffer(32);

}
void EspDrv::wifiDriverInit(UART_HandleTypeDef *_espUART)
{
	LOGDEBUG("> wifiDriverInit");

	m_espUART = _espUART;
	m_pSerial(m_espUART, 256, 256);

	bool initOK = false;

	for (int i = 0; i < 5; i++)
	{
		if (sendCmd("AT") == TAG_OK)
		{
			initOK = true;
			break;
		}
		HAL_Delay(1000);
	}

	if (!initOK)
	{
		LOGERROR("Cannot initialize ESP module");
		HAL_Delay(5000);
		return;
	}

	reset();

	// check firmware version
	getFwVersion();

	// prints a warning message if the firmware is not 1.X or 2.X
	if (((m_fwVersion[0] != '1') && (m_fwVersion[0] != '2'))
			|| (m_fwVersion[1] != '.'))
	{
		LOGWARN1("Warning: Unsupported firmware", m_fwVersion);
		HAL_Delay(4000);
	}
	else
	{
		LOGINFO1("Initilization successful -", m_fwVersion);
	}
}

void EspDrv::reset()
{
	LOGDEBUG("> reset");

	sendCmd("AT+RST");
	HAL_Delay(3000);
	espEmptyBuf(false);  // empty dirty characters from the buffer

	// disable echo of commands
	sendCmd("ATE0");

	// set station mode
	sendCmd("AT+CWMODE=1");
	HAL_Delay(200);

	// set multiple connections mode
	sendCmd("AT+CIPMUX=1");

	// Show remote IP and port with "+IPD"
	sendCmd("AT+CIPDINFO=1");

	// Disable auto connect
	// Automatic connection can create problems during initialization phase at next boot
	sendCmd("AT+CWAUTOCONN=0");

	// enable DHCP
	sendCmd("AT+CWDHCP=1,1");
	HAL_Delay(200);
}

bool EspDrv::wifiConnect(const char* ssid, const char* passphrase)
{
	LOGDEBUG("> wifiConnect");

	// TODO
	// Escape character syntax is needed if "SSID" or "password" contains
	// any special characters (',', '"' and '/')

	// connect to access point, use CUR mode to avoid connection at boot
	int ret = sendCmd("AT+CWJAP_CUR=\"%s\",\"%s\"", 20000, ssid, passphrase);

	if (ret == TAG_OK)
	{
		LOGINFO1("Connected to", ssid);
		return true;
	}

	LOGWARN1("Failed connecting to", ssid);

	// clean additional messages logged after the FAIL tag
	HAL_Delay(1000);
	espEmptyBuf(false);

	return false;
}

bool EspDrv::wifiStartAP(const char* ssid, const char* pwd, uint8_t channel,
		uint8_t enc, uint8_t espMode)
{
	LOGDEBUG("> wifiStartAP");

	// set AP mode, use CUR mode to avoid automatic start at boot
	int ret = sendCmd("AT+CWMODE_CUR=%d", 10000, espMode);
	if (ret != TAG_OK)
	{
		LOGWARN1("Failed to set AP mode", ssid);
		return false;
	}

	// TODO
	// Escape character syntax is needed if "SSID" or "password" contains
	// any special characters (',', '"' and '/')

	// start access point
	ret = sendCmd("AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d", 10000, ssid, pwd, channel,
			enc);

	if (ret != TAG_OK)
	{
		LOGWARN1("Failed to start AP", ssid);
		return false;
	}

	if (espMode == 2)
		sendCmd("AT+CWDHCP_CUR=0,1");    // enable DHCP for AP mode
	if (espMode == 3)
		sendCmd("AT+CWDHCP_CUR=2,1");  // enable DHCP for station and AP mode

	LOGINFO1("Access point started", ssid);
	return true;
}

int8_t EspDrv::disconnect()
{
	LOGDEBUG("> disconnect");

	if (sendCmd("AT+CWQAP") == TAG_OK)
		return WL_DISCONNECTED;

	// wait and clear any additional message
	HAL_Delay(2000);
	espEmptyBuf(false);

	return WL_DISCONNECTED;
}

void EspDrv::config(IPAddress& ip)
{
	LOGDEBUG("> config");

	// disable station DHCP
	sendCmd("AT+CWDHCP_CUR=1,0");

	// it seems we need to wait here...
	HAL_Delay(500);

	char buf[16];
	sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	int ret = sendCmd("AT+CIPSTA_CUR=\"%s\"", 2000, buf);
	HAL_Delay(500);

	if (ret == TAG_OK)
	{
		LOGINFO1("IP address set", buf);
	}
}

void EspDrv::configAP(IPAddress& ip)
{
	LOGDEBUG("> configAP");

	sendCmd("AT+CWMODE_CUR=2");

	// disable station DHCP
	sendCmd("AT+CWDHCP_CUR=2,0");

	// it seems we need to wait here...
	HAL_Delay(500);

	char buf[16];
	sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	int ret = sendCmd("AT+CIPAP_CUR=\"%s\"", 2000, buf);
	HAL_Delay(500);

	if (ret == TAG_OK)
	{
		LOGINFO1("IP address set", buf);
	}
}

uint8_t EspDrv::getConnectionStatus()
{
	LOGDEBUG("> getConnectionStatus");

	/*
	 AT+CIPSTATUS

	 Response

	 STATUS:<stat>
	 +CIPSTATUS:<link ID>,<type>,<remote_IP>,<remote_port>,<local_port>,<tetype>

	 Parameters

	 <stat>
	 2: Got IP
	 3: Connected
	 4: Disconnected
	 <link ID> ID of the connection (0~4), for multi-connect
	 <type> string, "TCP" or "UDP"
	 <remote_IP> string, remote IP address.
	 <remote_port> remote port number
	 <local_port> ESP8266 local port number
	 <tetype>
	 0: ESP8266 runs as client
	 1: ESP8266 runs as server
	 */

	char buf[10];
	if (!sendCmdGet("AT+CIPSTATUS", "STATUS:", "\r\n", buf, sizeof(buf)))
		return WL_NO_SHIELD;

	// 4: client disconnected
	// 5: wifi disconnected

	int s = atoi(buf);
	if (s == 2 or s == 3 or s == 4)
		return WL_CONNECTED;
	else if (s == 5)
		return WL_DISCONNECTED;

	return WL_IDLE_STATUS;
}

uint8_t EspDrv::getClientState(uint8_t sock)
{
	LOGDEBUG1D("> getClientState", sock);

	char findBuf[20];
	sprintf(findBuf, "+CIPSTATUS:%d,", sock);

	char buf[10];
	if (sendCmdGet("AT+CIPSTATUS", findBuf, ",", buf, sizeof(buf)))
	{
		LOGDEBUG("Connected");
		return true;
	}

	LOGDEBUG("Not connected");
	return false;
}

uint8_t* EspDrv::getMacAddress()
{
	LOGDEBUG("> getMacAddress");

	memset(m_mac, 0, WL_MAC_ADDR_LENGTH);

	char buf[20];
	if (sendCmdGet("AT+CIFSR", ":STAMAC,\"", "\"", buf, sizeof(buf)))
	{
		char* token;

		token = strtok(buf, ":");
		m_mac[5] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_mac[4] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_mac[3] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_mac[2] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_mac[1] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_mac[0] = (uint8_t) strtol(token, NULL, 16);
	}
	return m_mac;
}

void EspDrv::getIpAddress(IPAddress& ip)
{
	LOGDEBUG("> getIpAddress");

	char buf[20];
	memset(buf, '\0', sizeof(buf));
	if (sendCmdGet("AT+CIFSR", ":STAIP,\"", "\"", buf, sizeof(buf)))
	{
		char* token;

		token = strtok(buf, ".");
		m_localIp[0] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[1] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[2] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[3] = atoi(token);

		ip = m_localIp;
	}
}

void EspDrv::getIpAddressAP(IPAddress& ip)
{
	LOGDEBUG("> getIpAddressAP");

	char buf[20];
	memset(buf, '\0', sizeof(buf));
	if (sendCmdGet("AT+CIPAP?", "+CIPAP:ip:\"", "\"", buf, sizeof(buf)))
	{
		char* token;

		token = strtok(buf, ".");
		m_localIp[0] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[1] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[2] = atoi(token);
		token = strtok(NULL, ".");
		m_localIp[3] = atoi(token);

		ip = m_localIp;
	}
}

char* EspDrv::getCurrentSSID()
{
	LOGDEBUG("> getCurrentSSID");

	m_ssid[0] = 0;
	sendCmdGet("AT+CWJAP?", "+CWJAP:\"", "\"", m_ssid, sizeof(m_ssid));

	return m_ssid;
}

uint8_t* EspDrv::getCurrentBSSID()
{
	LOGDEBUG("> getCurrentBSSID");

	memset(m_bssid, 0, WL_MAC_ADDR_LENGTH);

	char buf[20];
	if (sendCmdGet("AT+CWJAP?", ",\"", "\",", buf, sizeof(buf)))
	{
		char* token;

		token = strtok(buf, ":");
		m_bssid[5] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_bssid[4] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_bssid[3] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_bssid[2] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_bssid[1] = (uint8_t) strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		m_bssid[0] = (uint8_t) strtol(token, NULL, 16);
	}
	return m_bssid;

}

int32_t EspDrv::getCurrentRSSI()
{
	LOGDEBUG("> getCurrentRSSI");

	int ret = 0;
	char buf[10];
	sendCmdGet("AT+CWJAP?", ",-", "\r\n", buf, sizeof(buf));

	if (isdigit(buf[0]))
	{
		ret = -atoi(buf);
	}

	return ret;
}

uint8_t EspDrv::getScanNetworks()
{
	uint8_t ssidListNum = 0;
	int idx;

	espEmptyBuf();

	LOGDEBUG("----------------------------------------------");
	LOGDEBUG(">> AT+CWLAP");

	println("AT+CWLAP");

	idx = readUntil(10000, "+CWLAP:(");

	while (idx == NUMESPTAGS)
	{
		m_networkEncr[ssidListNum] = m_pSerial->ParseInt();

		// discard , and " characters
		readUntil(1000, "\"");

		idx = readUntil(1000, "\"", false);
		if (idx == NUMESPTAGS)
		{
			memset(m_networkSsid[ssidListNum], 0, WL_SSID_MAX_LENGTH);
			m_ringBuf->getStrN(m_networkSsid[ssidListNum], 1,
					WL_SSID_MAX_LENGTH - 1);
		}

		// discard , character
		readUntil(1000, ",");

		m_networkRssi[ssidListNum] = m_pSerial->ParseInt();

		idx = readUntil(1000, "+CWLAP:(");

		if (ssidListNum == WL_NETWORKS_LIST_MAXNUM - 1)
			break;

		ssidListNum++;
	}

	if (idx == -1)
		return -1;

	LOGDEBUG1D("---------------------------------------------- >", ssidListNum);
	LOGDEBUG("\n");

	return ssidListNum;
}

bool EspDrv::getNetmask(IPAddress& mask)
{
	LOGDEBUG("> getNetmask");

	char buf[20];
	if (sendCmdGet("AT+CIPSTA?", "+CIPSTA:netmask:\"", "\"", buf, sizeof(buf)))
	{
		mask.fromString(buf);
		return true;
	}

	return false;
}

bool EspDrv::getGateway(IPAddress& gw)
{
	LOGDEBUG("> getGateway");

	char buf[20];
	if (sendCmdGet("AT+CIPSTA?", "+CIPSTA:gateway:\"", "\"", buf, sizeof(buf)))
	{
		gw.fromString(buf);
		return true;
	}

	return false;
}

char* EspDrv::getSSIDNetoworks(uint8_t networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return NULL;

	return m_networkSsid[networkItem];
}

uint8_t EspDrv::getEncTypeNetowrks(uint8_t networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return 0;

	return m_networkEncr[networkItem];
}

int32_t EspDrv::getRSSINetoworks(uint8_t networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return 0;

	return m_networkRssi[networkItem];
}

char* EspDrv::getFwVersion()
{
	LOGDEBUG("> getFwVersion");

	m_fwVersion[0] = 0;

	sendCmdGet("AT+GMR", "SDK version:", "\r\n", m_fwVersion,
			sizeof(m_fwVersion));

	return m_fwVersion;
}

bool EspDrv::ping(const char *host)
{
	LOGDEBUG("> ping");

	int ret = sendCmd("AT+PING=\"%s\"", 8000, host);

	if (ret == TAG_OK)
		return true;

	return false;
}

// Start server TCP on port specified
bool EspDrv::startServer(uint16_t port, uint8_t sock)
{
	LOGDEBUG1D("> startServer", port);

	int ret = sendCmd("AT+CIPSERVER=%d,%d", 1000, sock, port);

	return ret == TAG_OK;
}

bool EspDrv::startClient(const char* host, uint16_t port, uint8_t sock,
		uint8_t protMode)
{
	LOGDEBUG2SD("> startClient", host, port);

	// TCP
	// AT+CIPSTART=<link ID>,"TCP",<remote IP>,<remote port>

	// UDP
	// AT+CIPSTART=<link ID>,"UDP",<remote IP>,<remote port>[,<UDP local port>,<UDP mode>]

	// for UDP we set a dummy remote port and UDP mode to 2
	// this allows to specify the target host/port in CIPSEND

	int ret = -1;
	if (protMode == TCP_MODE)
		ret = sendCmd("AT+CIPSTART=%d,\"TCP\",\"%s\",%u", 5000, sock, host,
				port);
	else if (protMode == SSL_MODE)
	{
		// better to put the CIPSSLSIZE here because it is not supported before firmware 1.4
		sendCmd("AT+CIPSSLSIZE=4096");
		ret = sendCmd("AT+CIPSTART=%d,\"SSL\",\"%s\",%u", 5000, sock, host,
				port);
	}
	else if (protMode == UDP_MODE)
		ret = sendCmd("AT+CIPSTART=%d,\"UDP\",\"%s\",0,%u,2", 5000, sock, host,
				port);

	return ret == TAG_OK;
}

// Start server TCP on port specified
void EspDrv::stopClient(uint8_t sock)
{
	LOGDEBUG1D("> stopClient", sock);

	sendCmd("AT+CIPCLOSE=%d", 4000, sock);
}

uint8_t EspDrv::getServerState(uint8_t sock)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// TCP/IP functions
////////////////////////////////////////////////////////////////////////////

uint16_t EspDrv::availData(uint8_t connId)
{
	uint8_t buf[80];
	char c;

	// if there is data in the buffer
	if (m_pSerial->Available() != 0)
	{
		if (m_connId == connId)
			return m_pSerial->Available();
		else if (m_connId == 0)
			return m_pSerial->Available();
	}

	int bytes = m_pSerial->Available();

	if (bytes)
	{
		//LOGDEBUG1("Bytes in the serial buffer: ", bytes);
		//if (find((char *) "+IPD,"))
		{
			// format is : +IPD,<id>,<len>:<data>
			// format is : +IPD,<ID>,<len>[,<remote IP>,<remote port>]:<data>

			m_connId = m_pSerial->ParseInt();    // <ID>
			m_pSerial->Read(&c, 1);
			m_bufPos = m_pSerial->ParseInt();    // <len>
			m_pSerial->Read(&c, 1);
			m_remoteIp[0] = m_pSerial->ParseInt();    // <remote IP>
			m_pSerial->Read(&c, 1);
			m_remoteIp[1] = m_pSerial->ParseInt();
			m_pSerial->Read(&c, 1);
			m_remoteIp[2] = m_pSerial->ParseInt();
			m_pSerial->Read(&c, 1);
			m_remoteIp[3] = m_pSerial->ParseInt();
			m_pSerial->Read(&c, 1);
			m_pSerial->Read(&c, 1);
			m_remotePort = m_pSerial->ParseInt();     // <remote port>

			m_pSerial->Read(&c, 1);

			LOGDEBUG("");
			LOGDEBUG2DL("Data packet", m_connId, m_bufPos);

			if (m_connId == connId || connId == 0)
				return m_bufPos;
		}
	}
	return 0;

}

bool EspDrv::getData(uint8_t connId, uint8_t *data, bool peek, bool* connClose)
{
	if (connId != m_connId)
		return false;

	long startTick = HAL_GetTick();
	do
	{
		if (m_pSerial->Available())
		{
			if (peek)
			{
				*data = (char) m_pSerial->Peek();
			}
			else
			{
				*data = (char) espSerial->GetChar();
			}
			//Serial.print((char)*data);

			if (m_pSerial->Available())
			{
				// after the data packet a ",CLOSED" string may be received
				// this means that the socket is now closed

				HAL_Delay(5);

				if (m_pSerial->Available())
				{
					//LOGDEBUG(".2");
					//LOGDEBUG(espSerial->peek());

					// 48 = '0'
					if (m_pSerial->Peek() == ('0' + connId))	// QD convert to ASCII
					{
						int idx = readUntil(500, ",CLOSED\r\n", false);
						if (idx != NUMESPTAGS)
						{
							LOGERROR(F("Tag CLOSED not found"));
						}

						LOGDEBUG();
						LOGDEBUG(F("Connection closed"));

						*connClose = true;
					}
				}
			}

			return true;
		}
	} while ((HAL_GetTick() - startTick) < 2000);

	// timed out, reset the buffer
	LOGERROR1(F("TIMEOUT:"), m_pSerial->Available());

	m_connId = 0;
	*data = 0;

	return false;
}

/**
 * Receive the data into a buffer.
 * It reads up to bufSize bytes.
 * @return	received data size for success else -1.
 */
int EspDrv::getDataBuf(uint8_t connId, uint8_t *buf, uint16_t bufSize)
{
	if (connId != m_connId)
		return false;

	bufSize = m_pSerial->Read(buf, bufSize, 0);	// read no wait

	return bufSize;
}

bool EspDrv::sendData(uint8_t sock, const uint8_t *data, uint16_t len)
{
	char cmdBuf[32];

	LOGDEBUG2DD("> sendData:", sock, len);

	sprintf(cmdBuf, "AT+CIPSEND=%d,%u\n", sock, len);
	m_pSerial->Write(cmdBuf, sizeof(cmdBuf));

	// read chars until we see '>'

	int idx = readUntil(1000, (char *) ">", false);

	// if something other timed out first then something other than NUMESPTAGS is returned for an error
	if (idx != NUMESPTAGS)
	{
		LOGERROR("Data packet send error (1)");
		return false;
	}

	m_pSerial->Write(data, len, 1000);

	idx = readUntil(2000);
	if (idx != TAG_SENDOK)
	{
		LOGERROR("Data packet send error (2)");
		return false;
	}

	return true;
}

// Overloaded -- sendData method
bool EspDrv::sendData(uint8_t sock, const char *data, uint16_t len,
		bool appendCrLf)
{
	char cmdBuf[32];

	LOGDEBUG2DD("> sendData:", sock, len);

	sprintf(cmdBuf, "AT+CIPSEND=%d,%u\n", sock, len2);
	m_pSerial->Write(cmdBuf, strlen(cmdBuf));

	int idx = readUntil(1000, (char *) ">", false);

	if (idx != NUMESPTAGS)
	{
		LOGERROR("Data packet send error (1)");
		return false;
	}

	m_pSerial->Write(data, len, 1000);

	if (appendCrLf)
	{
		sprintf(cmdBuf, "\r\n");
		m_pSerial->Write(cmdBuf, strlen(cmdBuf));
	}

	idx = readUntil(2000);
	if (idx != TAG_SENDOK)
	{
		LOGERROR("Data packet send error (2)");
		return false;
	}

	return true;
}

bool EspDrv::sendDataUdp(uint8_t sock, const char* host, uint16_t port,
		const uint8_t *data, uint16_t len)
{
	LOGDEBUG2DD("> sendDataUdp:", sock, len);
	LOGDEBUG2SD("> sendDataUdp:", host, port);

	char cmdBuf[40];
	sprintf(cmdBuf, "AT+CIPSEND=%d,%u,\"%s\",%u\n", sock, len, host, port);
	m_pSerial->Write(cmdBuf, strlen(cmdBuf));

	int idx = readUntil(1000, (char *) ">", false);
	if (idx != NUMESPTAGS)
	{
		LOGERROR("Data packet send error (1)");
		return false;
	}

	m_Serial->Write(data, len);

	idx = readUntil(2000);
	if (idx != TAG_SENDOK)
	{
		LOGERROR("Data packet send error (2)");
		return false;
	}

	return true;
}

void EspDrv::getRemoteIpAddress(IPAddress& ip)
{
	ip = m_remoteIp;
}

uint16_t EspDrv::getRemotePort()
{
	return m_remotePort;
}

////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////

/*
 * Sends the AT command and stops if any of the TAGS is found.
 * Extract the string enclosed in the passed tags and returns it in the outStr buffer.
 * Returns true if the string is extracted, false if tags are not found of timed out.
 */
bool EspDrv::sendCmdGet(const char* cmd, const char* startTag,
		const char* endTag, char* outStr, int outStrLen)
{
	int idx;
	bool ret = false;

	outStr[0] = 0;

	espEmptyBuf();

	LOGDEBUG("----------------------------------------------");
	LOGDEBUG1(">>", cmd);

	// send AT command to ESP
	//espSerial->println(cmd);

	// read result until the startTag is found
	idx = readUntil(1000, startTag);

	if (idx == NUMESPTAGS)
	{
		// clean the buffer to get a clean string
		//m_ringBuf->init();

		// start tag found, search the endTag
		idx = readUntil(500, endTag);

		if (idx == NUMESPTAGS)
		{
			// end tag found
			// copy result to output buffer avoiding overflow
			m_ringBuf->getStrN(outStr, strlen(endTag), outStrLen - 1);

			// read the remaining part of the response
			readUntil(2000);

			ret = true;
		}
		else
		{
			LOGWARN("End tag not found");
		}
	}
	else if (idx >= 0 and idx < NUMESPTAGS)
	{
		// the command has returned but no start tag is found
		LOGDEBUG1D("No start tag found:", idx);
	}
	else
	{
		// the command has returned but no tag is found
		LOGWARN("No tag found");
	}

	LOGDEBUG1("---------------------------------------------- >", outStr);
	LOGDEBUG("");

	return ret;
}

/*
 * Sends the AT command and returns the id of the TAG.
 * Return -1 if no tag is found.
 */
int EspDrv::sendCmd(const char* cmd, int timeout)
{
	HAL_StatusTypeDef status;
	espEmptyBuf();

	LOGDEBUG("----------------------------------------------");
	LOGDEBUG1(">>", cmd);

	int status = m_pSerial->Write((uint8_t*) cmd, strlen(cmd), timeout);

	int idx = readUntil(timeout);

	LOGDEBUG1D("---------------------------------------------- >", idx);
	LOGDEBUG("");

	return idx;
}

/*
 * Sends the AT command and returns the id of the TAG.
 * The additional arguments are formatted into the command using sprintf.
 * Return -1 if no tag is found.
 */
int EspDrv::sendCmd(const char* cmd, int timeout, ...)
{
	char cmdBuf[CMD_BUFFER_SIZE];

	va_list args;
	va_start(args, timeout);
	vsnprintf(cmdBuf, CMD_BUFFER_SIZE, (char*) cmd, args);
	va_end(args);

	espEmptyBuf();

	LOGDEBUG("----------------------------------------------");
	LOGDEBUG1(">>", cmdBuf);

	println(cmdBuf);

	int idx = readUntil(timeout);

	LOGDEBUG1D("---------------------------------------------- >", idx);
	LOGDEBUG("");

	return idx;
}

// Read from serial until one of the tags is found
// Returns:
//   the index of the tag found in the ESPTAGS array
//   -1 if no tag was found (timeout)
int EspDrv::readUntil(unsigned int timeout, const char* tag, bool findTags)
{
	m_pRingBuf->reset();

	char c;
	unsigned long start = HAL_GetTick();
	int ret = -1;

	while (((HAL_GetTick() - start) < timeout) && (ret < 0))
	{
		if (m_pSerial->Available() != 0)
		{
			c = m_pSerial->GetChar();
			// c = (char) m_espSerial->read();
			LOGDEBUG0C(c);

			if (tag != NULL)
			{
				if (m_ringBuf->endsWith(tag))
				{
					ret = NUMESPTAGS;
					//LOGDEBUG1("xxx");
				}
			}
			if (findTags)
			{
				for (int i = 0; i < NUMESPTAGS; i++)
				{
					if (m_ringBuf->endsWith(ESPTAGS[i]))
					{
						ret = i;
						break;
					}
				}
			}
		}
	}

	if ((HAL_GetTick() - start) >= timeout)
	{
		LOGWARN(">>> TIMEOUT >>>");
	}

	return ret;
}

void EspDrv::espEmptyBuf(bool warn)
{
	int i = 0;

	HAL_Delay(1000);
	i = m_pSerial->Available();

	if (i > 0 and warn == true)
	{
		LOGDEBUG("");
		LOGDEBUG1D("Dirty characters in the serial buffer! >", i);
	}
	m_pSerial->Flush();
}

bool EspDrv::available(void)
{
	return (m_pSerial->Available() > 0);
}
char EspDrv::read(uint32_t timeout)
{
	uint8_t buf[4];

	if (m_pSerial->Read(buf, 1, timeout) > 0)
		return buf[0];

	return -1;
}

int EspDrv::print(const char* str, uint32_t timeout)
{
	m_pSerial->Write((uint8_t*) str, strlen(str), timeout);
	return strlen(str);
}

int EspDrv::println(const char *str, uint32_t timeout)
{
	m_pSerial->Write((uint8_t*) str, strlen(str), timeout);
	char newline[3] = "\r\n";
	m_pSerial->Write((uint8_t *) newline, 2, timeout);

	return strlen(str) + 2;
}

