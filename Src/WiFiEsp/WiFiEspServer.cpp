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

#include "WiFiEspServer.h"
#include "WiFiEsp.h"

#include "utility/EspDrv.h"
#include "utility/debug.h"



WiFiEspServer::WiFiEspServer(uint16_t port, WiFiEspClass *_wifi)
{
	m_port = port;
	m_wifi = _wifi;
}

void WiFiEspServer::begin()
{
	LOGDEBUG("Starting server");

	/* The ESP Module only allows socket 1 to be used for the server */
#if 0
	m_sock = WiFiEspClass::getFreeSocket();
	if (m_sock == SOCK_NOT_AVAIL)
	  {
	    LOGERROR("No socket available for server");
	    return;
	  }
#else
	m_sock = 1; // If this is already in use, the startServer attempt will fail
#endif
	m_wifi->allocateSocket(m_sock);

	m_started = m_wifi->GetDrv()->startServer(m_port, m_sock);

	if (m_started)
	{
		LOGINFO1D("Server started on port", m_port);
	}
	else
	{
		LOGERROR("Server failed to start");
	}
}

WiFiEspClient WiFiEspServer::available(uint8_t* status)
{
	// TODO the original method seems to handle automatic server restart
	EspDrv *espDrv = m_wifi->GetDrv();

	int bytes = espDrv->availData(0);
	if (bytes > 0)
	{
		LOGINFO1D("New client", espDrv->m_connId);
		m_wifi->allocateSocket(espDrv->m_connId);
		WiFiEspClient client(espDrv->m_connId);
		return client;
	}

    return WiFiEspClient(255);
}

uint8_t WiFiEspServer::status()
{
    return EspDrv::getServerState(0);
}

size_t WiFiEspServer::write(uint8_t b)
{
    return write(&b, 1);
}

size_t WiFiEspServer::write(const uint8_t *buffer, size_t size)
{
	size_t n = 0;

    for (int sock = 0; sock < MAX_SOCK_NUM; sock++)
    {
        if (m_wifi->m_state[sock] != 0)
        {
        	WiFiEspClient client(sock);
            n += client.write(buffer, size);
        }
    }
    return n;
}
