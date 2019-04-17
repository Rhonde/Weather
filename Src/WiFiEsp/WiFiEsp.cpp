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

#include "WiFiEsp.h"

WiFiEspClass::WiFiEspClass(GPIO_TypeDef *_reset_port, uint32_t _reset_pin, GPIO_TypeDef *_ena_port, uint32_t _ena_pin)
{
	m_resetPort = _reset_port;
	m_resetPin = _reset_pin;
	m_enaPort = _ena_port;
	m_enaPin = _ena_pin;
	m_espMode = 0;
	m_espDrv = new EspDrv();
	for (int ix = 0; ix < MAX_SOCK_NUM; ix++)
	{
		m_state[ix] = NA_STATE;
		m_server_port[ix] = 0;
	}
}

void WiFiEspClass::init(UART_HandleTypeDef* _espUART)
{
	LOGINFO("Initializing ESP module");

	HAL_GPIO_WritePin(m_enaPort, m_enaPin, GPIO_PIN_SET);// set enable pin to '1'
	HAL_GPIO_WritePin(m_resetPort, m_resetPin, GPIO_PIN_RESET);	// reset low
	HAL_Delay(10);			// leave it low for 10 mS (TODO: find docs on this)
	HAL_GPIO_WritePin(m_resetPort, m_resetPin, GPIO_PIN_SET);	// reset hi
	m_espUART = _espUART;

	m_espDrv->wifiDriverInit(m_espUART);
}

char* WiFiEspClass::firmwareVersion()
{
	return m_espDrv->getFwVersion();
}

int WiFiEspClass::begin(const char* ssid, const char* passphrase)
{
	m_espMode = 1;
	if (m_espDrv->wifiConnect(ssid, passphrase))
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}

int WiFiEspClass::beginAP(const char* ssid, uint8_t channel, const char* pwd,
		uint8_t enc, bool apOnly)
{
	if (apOnly)
		m_espMode = 2;
	else
		m_espMode = 3;

	if (m_espDrv->wifiStartAP(ssid, pwd, channel, enc, m_espMode))
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}

int WiFiEspClass::beginAP(const char* ssid)
{
	return beginAP(ssid, 10, "", 0);
}

int WiFiEspClass::beginAP(const char* ssid, uint8_t channel)
{
	return beginAP(ssid, channel, "", 0);
}

void WiFiEspClass::config(IPAddress ip)
{
	m_espDrv->config(ip);
}

void WiFiEspClass::configAP(IPAddress ip)
{
	m_espDrv->configAP(ip);
}

int WiFiEspClass::disconnect()
{
	return m_espDrv->disconnect();
}

uint8_t* WiFiEspClass::macAddress(uint8_t* mac)
{
	// TODO we don't need _mac variable
	uint8_t* _mac = m_espDrv->getMacAddress();
	memcpy(mac, _mac, WL_MAC_ADDR_LENGTH);
	return mac;
}

IPAddress WiFiEspClass::localIP()
{
	IPAddress ret;
	if (m_espMode == 1)
		m_espDrv->getIpAddress(ret);
	else
		m_espDrv->getIpAddressAP(ret);
	return ret;
}

IPAddress WiFiEspClass::subnetMask()
{
	IPAddress mask;
	if (m_espMode == 1)
		m_espDrv->getNetmask(mask);
	return mask;
}

IPAddress WiFiEspClass::gatewayIP()
{
	IPAddress gw;
	if (m_espMode == 1)
		m_espDrv->getGateway(gw);
	return gw;
}

char* WiFiEspClass::SSID()
{
	return m_espDrv->getCurrentSSID();
}

uint8_t* WiFiEspClass::BSSID(uint8_t* bssid)
{
	// TODO we don't need _bssid
	uint8_t* _bssid = m_espDrv->getCurrentBSSID();
	memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
	return bssid;
}

int32_t WiFiEspClass::RSSI()
{
	return m_espDrv->getCurrentRSSI();
}

int8_t WiFiEspClass::scanNetworks()
{
	return m_espDrv->getScanNetworks();
}

char* WiFiEspClass::SSID(uint8_t networkItem)
{
	return m_espDrv->getSSIDNetoworks(networkItem);
}

int32_t WiFiEspClass::RSSI(uint8_t networkItem)
{
	return m_espDrv->getRSSINetoworks(networkItem);
}

uint8_t WiFiEspClass::encryptionType(uint8_t networkItem)
{
	return m_espDrv->getEncTypeNetowrks(networkItem);
}

uint8_t WiFiEspClass::status()
{
	return m_espDrv->getConnectionStatus();
}

////////////////////////////////////////////////////////////////////////////
// Non standard methods
////////////////////////////////////////////////////////////////////////////

void WiFiEspClass::reset(void)
{
	m_espDrv->reset();
}

/*
 void ESP8266::hardReset(void)
 {
 connected = false;
 strcpy(ip, "");
 digitalWrite(ESP8266_RST, LOW);
 delay(ESP8266_HARD_RESET_DURATION);
 digitalWrite(ESP8266_RST, HIGH);
 delay(ESP8266_HARD_RESET_DURATION);
 }
 */

bool WiFiEspClass::ping(const char *host)
{
	return m_espDrv->ping(host);
}

uint8_t WiFiEspClass::getFreeSocket()
{
	// ESP Module assigns socket numbers in ascending order, so we will assign them in descending order
	for (int i = MAX_SOCK_NUM - 1; i >= 0; i--)
	{
		if (m_state[i] == NA_STATE)
		{
			return i;
		}
	}
	return SOCK_NOT_AVAIL;
}

void WiFiEspClass::allocateSocket(uint8_t sock)
{
	m_state[sock] = sock;
}

void WiFiEspClass::releaseSocket(uint8_t sock)
{
	m_state[sock] = NA_STATE;
}


