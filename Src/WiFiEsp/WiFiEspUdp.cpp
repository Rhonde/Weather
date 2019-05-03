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
#include "WiFiEspUdp.h"

#include "utility/EspDrv.h"
#include "utility/debug.h"

/* Constructor */
WiFiEspUDP::WiFiEspUDP(WiFiEspClass *wifi)
{
	m_sock = NO_SOCKET_AVAIL;
	m_wifi = wifi;
	m_espDrv = wifi->GetDrv();
}




/* Start WiFiUDP socket, listening at local port PORT */

uint8_t WiFiEspUDP::begin(uint16_t port)
{
    uint8_t sock = m_wifi->getFreeSocket();
    if (sock != NO_SOCKET_AVAIL)
    {
        m_espDrv->startClient("0", port, sock, UDP_MODE);
		
        m_wifi->allocateSocket(sock);  // allocating the socket for the listener
        m_wifi->m_server_port[sock] = port;
        m_sock = sock;
        m_port = port;
        return 1;
    }
    return 0;

}


/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiEspUDP::available()
{
	 if (m_sock != NO_SOCKET_AVAIL)
	 {
		int bytes = m_wifi->GetDrv()->availData(m_sock);
		if (bytes>0)
		{
			return bytes;
		}
	}

	return 0;
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiEspUDP::stop()
{
	  if (m_sock == NO_SOCKET_AVAIL)
	    return;

      // Discard data that might be in the incoming buffer
      flush();
      
      // Stop the listener and return the socket to the pool
	  m_wifi->GetDrv()->stopClient(m_sock);
      m_wifi->m_state[m_sock] = NA_STATE;
      m_wifi->m_server_port[m_sock] = 0;

	  m_sock = NO_SOCKET_AVAIL;
}

int WiFiEspUDP::beginPacket(const char *host, uint16_t port)
{
  if (m_sock == NO_SOCKET_AVAIL)
	  m_sock = m_wifi->getFreeSocket();
  if (m_sock != NO_SOCKET_AVAIL)
  {
	  //EspDrv::startClient(host, port, m_sock, UDP_MODE);
	  m_remotePort = port;
	  strcpy(m_remoteHost, host);
	  m_wifi->allocateSocket(m_sock);
	  return 1;
  }
  return 0;
}


int WiFiEspUDP::beginPacket(IPAddress ip, uint16_t port)
{
	char s[18];
	sprintf_P(s, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);

	return beginPacket(s, port);
}


int WiFiEspUDP::endPacket()
{
	return 1; //ServerDrv::sendUdpData(m_sock);
}

size_t WiFiEspUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t WiFiEspUDP::write(const uint8_t *buffer, size_t size)
{
	bool r = m_wifi->GetDrv()->sendDataUdp(m_sock, m_remoteHost, m_remotePort, buffer, size);
	if (!r)
	{
		return 0;
	}

	return size;
}

int WiFiEspUDP::parsePacket()
{
	return available();
}

int WiFiEspUDP::read()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	
    // Read the data and handle the timeout condition
	if (! m_wifi->GetDrv()->getData(m_sock, &b, false, &connClose))
      return -1;  // Timeout occured

	return b;
}

int WiFiEspUDP::read(uint8_t* buf, size_t size)
{
	if (!available())
		return -1;
	return m_wifi->GetDrv()->getDataBuf(m_sock, buf, size);
}

int WiFiEspUDP::peek()
{
  uint8_t b=0;
  if (!available())
    return -1;

  return b;
}

void WiFiEspUDP::flush()
{
	  // Discard all input data
	  int count = available();
	  while (count-- > 0)
	    read();
}


IPAddress  WiFiEspUDP::remoteIP()
{
	IPAddress ret;
	m_wifi->GetDrv()->getRemoteIpAddress(ret);
	return ret;
}

uint16_t  WiFiEspUDP::remotePort()
{
	return m_wifi->GetDrv()->getRemotePort();
}



////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////


