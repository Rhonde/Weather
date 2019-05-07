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

#include <inttypes.h>

#include "WiFiEsp.h"
#include "WiFiEspClient.h"
#include "WiFiEspServer.h"

#include "utility/EspDrv.h"
#include "utility/debug.h"

WiFiEspClient::WiFiEspClient(WiFiEspClass* _wifi)
{
	m_sock = 255;
	m_wifi = _wifi;
}

WiFiEspClient::WiFiEspClient(WiFiEspClass* _wifi, uint8_t sock)
{
	m_wifi = _wifi;
	m_sock = sock;
}

////////////////////////////////////////////////////////////////////////////////
// Override Print methods
////////////////////////////////////////////////////////////////////////////////

// the standard print method will call write for each character in the buffer
// this is very slow on ESP
size_t WiFiEspClient::print(const char *str)
{
	printf(str);
	return strlen(str);
}

// if we do override this, the standard println will call the print
// method twice
size_t WiFiEspClient::println(const char *str)
{
	printf(str);
	printf("\n");
	return strlen(str);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of Client virtual methods
////////////////////////////////////////////////////////////////////////////////

int WiFiEspClient::connectSSL(const char* host, uint16_t port)
{
	return connect(host, port, SSL_MODE);
}

int WiFiEspClient::connectSSL(IPAddress ip, uint16_t port)
{
	char s[16];
	sprintf(s, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return connect(s, port, SSL_MODE);
}

int WiFiEspClient::connect(const char* host, uint16_t port)
{
	return connect(host, port, TCP_MODE);
}

int WiFiEspClient::connect(const IPAddress &ip, uint16_t port)
{
	char s[16];
	sprintf(s, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	return connect(s, port, TCP_MODE);
}

/* Private method */
int WiFiEspClient::connect(const char* host, uint16_t port, uint8_t protMode)
{
	LOGINFO1("Connecting to", host);

	m_sock = m_wifi->getFreeSocket();

	if (m_sock != NO_SOCKET_AVAIL)
	{

		if (m_wifi->GetDrv()->startClient(host, port, m_sock, protMode))
			return 0;

		m_wifi->allocateSocket(m_sock);
	}
	else
	{
		LOGERROR("No socket available");
		return 0;
	}
	return 1;
}

size_t WiFiEspClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t WiFiEspClient::write(const uint8_t *buf, size_t size)
{
	if (m_sock >= MAX_SOCK_NUM or size == 0)
	{
		setWriteError();
		return 0;
	}

	bool r = m_wifi->GetDrv()->sendData(m_sock, buf, size);
	if (!r)
	{
		setWriteError();
		LOGERROR1D("Failed to write to socket", m_sock);
		HAL_Delay(4000);
		stop();
		return 0;
	}

	return size;
}

int WiFiEspClient::available()
{
	if (m_sock != 255)
	{
		int bytes = m_wifi->GetDrv()->availData(m_sock);
		if (bytes > 0)
		{
			return bytes;
		}
	}

	return 0;
}

int WiFiEspClient::read()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	m_wifi->GetDrv()->getData(m_sock, &b, false, &connClose);

	if (connClose)
	{
		m_wifi->releaseSocket(m_sock);
		m_sock = 255;
	}

	return b;
}

int WiFiEspClient::read(uint8_t* buf, size_t size)
{
	if (!available())
		return -1;
	return m_wifi->GetDrv()->getDataBuf(m_sock, buf, size);
}

int WiFiEspClient::peek()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	m_wifi->GetDrv()->getData(m_sock, &b, true, &connClose);

	if (connClose)
	{
		m_wifi->releaseSocket(m_sock);
		m_sock = 255;
	}

	return b;
}

bool WiFiEspClient::flush(unsigned int maxWaitMs)
{
	int timer = HAL_GetTick();

	while (available())
	{
		if ((HAL_GetTick() - timer) > maxWaitMs)
			return false;
		read();
	}

	return true;
}

bool WiFiEspClient::stop(unsigned int maxWaitMs)
{
	if (m_sock == 255)
		return true;

	LOGINFO1D("Disconnecting ", m_sock);

	m_wifi->GetDrv()->stopClient(m_sock);

	m_wifi->releaseSocket(m_sock);
	m_sock = 255;
	return true;
}

uint8_t WiFiEspClient::connected()
{
	return (status() == ESTABLISHED);
}

WiFiEspClient::operator bool()
{
	return m_sock != 255;
}

////////////////////////////////////////////////////////////////////////////////
// Additional WiFi standard methods
////////////////////////////////////////////////////////////////////////////////

uint8_t WiFiEspClient::status()
{
	if (m_sock == 255)
	{
		return CLOSED;
	}

	if (m_wifi->GetDrv()->availData(m_sock))
	{
		return ESTABLISHED;
	}

	if (m_wifi->GetDrv()->getClientState(m_sock))
	{
		return ESTABLISHED;
	}

	m_wifi->releaseSocket(m_sock);
	m_sock = 255;

	return CLOSED;
}

IPAddress WiFiEspClient::remoteIP()
{
	IPAddress ret;
	m_wifi->GetDrv()->getRemoteIpAddress(ret);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////

size_t WiFiEspClient::print(const char *ifsh, bool appendCrLf)
{
	size_t size = strlen_P((char*) ifsh);

	if (m_sock >= MAX_SOCK_NUM or size == 0)
	{
		setWriteError();
		return 0;
	}

	bool r = m_wifi->GetDrv()->sendData(m_sock, ifsh, size, appendCrLf);
	if (!r)
	{
		setWriteError();
		LOGERROR1D("Failed to write to socket", m_sock);
		HAL_Delay(4000);
		stop();
		return 0;
	}

	return size;
}
