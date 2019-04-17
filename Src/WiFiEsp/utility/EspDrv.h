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

#ifndef EspDrv_h
#define EspDrv_h

#include <ctype.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>

#include "Stream.h"
#include "IPAddress.h"


#include "RingBuffer.h"



// Maximum size of a SSID
#define WL_SSID_MAX_LENGTH 32

// Size of a MAC-address or BSSID
#define WL_MAC_ADDR_LENGTH 6

// Size of a MAC-address or BSSID
#define WL_IPV4_LENGTH 4

// Maximum size of a SSID list
#define WL_NETWORKS_LIST_MAXNUM	10

// Maxmium number of socket
#define	MAX_SOCK_NUM		4

// Socket not available constant
#define SOCK_NOT_AVAIL  255

// Default state value for Wifi state field
#define NA_STATE -1

#define WL_FW_VER_LENGTH 6

#define NO_SOCKET_AVAIL 255


// maximum size of AT command
#define CMD_BUFFER_SIZE 200


typedef enum eProtMode {TCP_MODE, UDP_MODE, SSL_MODE} tProtMode;


typedef enum {
        WL_FAILURE = -1,
        WL_SUCCESS = 1,
} wl_error_code_t;

/* Authentication modes */
enum wl_auth_mode {
        AUTH_MODE_INVALID,
        AUTH_MODE_AUTO,
        AUTH_MODE_OPEN_SYSTEM,
        AUTH_MODE_SHARED_KEY,
        AUTH_MODE_WPA,
        AUTH_MODE_WPA2,
        AUTH_MODE_WPA_PSK,
        AUTH_MODE_WPA2_PSK
};


typedef enum {
	WL_NO_SHIELD = 255,
	WL_IDLE_STATUS = 0,
	//WL_NO_SSID_AVAIL,
	//WL_SCAN_COMPLETED,
	WL_CONNECTED,
	WL_CONNECT_FAILED,
	//WL_CONNECTION_LOST,
	WL_DISCONNECTED
} wl_status_t;

/* Encryption modes */
enum wl_enc_type {
	ENC_TYPE_NONE = 0,
	ENC_TYPE_WEP = 1,
	ENC_TYPE_WPA_PSK = 2,
	ENC_TYPE_WPA2_PSK = 3,
	ENC_TYPE_WPA_WPA2_PSK = 4
};


enum wl_tcp_state {
	CLOSED      = 0,
	LISTEN      = 1,
	SYN_SENT    = 2,
	SYN_RCVD    = 3,
	ESTABLISHED = 4,
	FIN_WAIT_1  = 5,
	FIN_WAIT_2  = 6,
	CLOSE_WAIT  = 7,
	CLOSING     = 8,
	LAST_ACK    = 9,
	TIME_WAIT   = 10
};



class EspDrv
{

public:
	EspDrv();
    void wifiDriverInit(UART_HandleTypeDef *espUART);


    /* Start Wifi connection with passphrase
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase must be between ASCII 32-126 (decimal).
     */
    bool wifiConnect(const char* ssid, const char* passphrase);


    /*
	* Start the Access Point
	*/
	bool wifiStartAP(const char* ssid, const char* pwd, uint8_t channel, uint8_t enc, uint8_t espMode);


    /*
	 * Set ip configuration disabling dhcp client
	 */
    void config(IPAddress& local_ip);

    /*
	 * Set ip configuration disabling dhcp client
	 */
    void configAP(IPAddress& local_ip);


    /*
     * Disconnect from the network
     *
     * return: WL_SUCCESS or WL_FAILURE
     */
    int8_t disconnect();

    /*
     *
     *
     * return: one value of wl_status_t enum
     */
    uint8_t getConnectionStatus();

    /*
     * Get the interface MAC address.
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* getMacAddress();

    /*
     * Get the interface IP address.
     *
     * return: copy the ip address value in IPAddress object
     */
    void getIpAddress(IPAddress& ip);

	void getIpAddressAP(IPAddress& ip);

    /*
     * Get the interface IP netmask.
     * This can be used to retrieve settings configured through DHCP.
     *
     * return: true if successful
     */
    bool getNetmask(IPAddress& mask);

    /*
     * Get the interface IP gateway.
     * This can be used to retrieve settings configured through DHCP.
     *
     * return: true if successful
     */
    bool getGateway(IPAddress& mask);

    /*
     * Return the current SSID associated with the network
     *
     * return: ssid string
     */
    char* getCurrentSSID();

    /*
     * Return the current BSSID associated with the network.
     * It is the MAC address of the Access Point
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* getCurrentBSSID();

    /*
     * Return the current RSSI /Received Signal Strength in dBm)
     * associated with the network
     *
     * return: signed value
     */
    int32_t getCurrentRSSI();

    /*
     * Get the networks available
     *
     * return: Number of discovered networks
     */
    uint8_t getScanNetworks();

	/*
     * Return the SSID discovered during the network scan.
     *
     * param networkItem: specify from which network item want to get the information
	 *
     * return: ssid string of the specified item on the networks scanned list
     */
    char* getSSIDNetoworks(uint8_t networkItem);

    /*
     * Return the RSSI of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
	 *
     * return: signed value of RSSI of the specified item on the networks scanned list
     */
    int32_t getRSSINetoworks(uint8_t networkItem);

    /*
     * Return the encryption type of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
	 *
     * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
     */
    uint8_t getEncTypeNetowrks(uint8_t networkItem);


    /*
     * Get the firmware version
     */
    char* getFwVersion();


	////////////////////////////////////////////////////////////////////////////
	// Client/Server methods
	////////////////////////////////////////////////////////////////////////////


    bool startServer(uint16_t port, uint8_t sock);
    bool startClient(const char* host, uint16_t port, uint8_t sock, uint8_t protMode);
    void stopClient(uint8_t sock);
    uint8_t getServerState(uint8_t sock);
    uint8_t getClientState(uint8_t sock);
    bool getData(uint8_t connId, uint8_t *data, bool peek, bool* connClose);
    int getDataBuf(uint8_t connId, uint8_t *buf, uint16_t bufSize);
    bool sendData(uint8_t sock, const uint8_t *data, uint16_t len);
    bool sendData(uint8_t sock, const __FlashStringHelper *data, uint16_t len, bool appendCrLf=false);
	bool sendDataUdp(uint8_t sock, const char* host, uint16_t port, const uint8_t *data, uint16_t len);
    uint16_t availData(uint8_t connId);

	bool ping(const char *host);
    void reset();

    void getRemoteIpAddress(IPAddress& ip);
    uint16_t getRemotePort();
    uint8_t getConnId() {return m_connId;};

////////////////////////////////////////////////////////////////////////////////

private:
	UART_HandleTypeDef *m_espUART;

	long m_bufPos;
	uint8_t m_connId;

	uint16_t m_remotePort;
	uint8_t  m_remoteIp[WL_IPV4_LENGTH];


	// firmware version string
	char 	m_fwVersion[WL_FW_VER_LENGTH];

	// settings of requested network
	char 	m_networkSsid[WL_NETWORKS_LIST_MAXNUM][WL_SSID_MAX_LENGTH];
	int32_t 	m_networkRssi[WL_NETWORKS_LIST_MAXNUM];
	uint8_t 	m_networkEncr[WL_NETWORKS_LIST_MAXNUM];


	// settings of current selected network
	char 	m_ssid[WL_SSID_MAX_LENGTH];
	uint8_t m_bssid[WL_MAC_ADDR_LENGTH];
	uint8_t m_mac[WL_MAC_ADDR_LENGTH];
	uint8_t m_localIp[WL_IPV4_LENGTH];


	// the ring buffer is used to search the tags in the stream
	RingBuffer* m_ringBuf;


	//static int sendCmd(const char* cmd, int timeout=1000);
	int sendCmd(const char* cmd, int timeout=1000);
	int sendCmd(const char* cmd, int timeout, ...);

	bool sendCmdGet(const char* cmd, const char* startTag, const char* endTag, char* outStr, int outStrLen);

	int readUntil(unsigned int timeout, const char* tag=NULL, bool findTags=true);

	void espEmptyBuf(bool warn=true);

	int timedRead();
	int print(const char *str, uint32_t timeout = 1000);
	int println(const char *str, uint32_t timeout = 1000);
	bool available(void);
	char read(int timeout);


	friend class WiFiEsp;
	friend class WiFiEspServer;
	friend class WiFiEspClient;
	friend class WiFiEspUdp;
};

#endif
