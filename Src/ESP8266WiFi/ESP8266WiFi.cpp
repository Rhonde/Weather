/*
 ESP8266WiFi.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Reworked on 28 Dec 2015 by Markus Sattler

 */

#include "ESP8266WiFi.h"

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------- Debug ------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

/**
 * Output WiFi settings to an object derived from Print interface (like Serial).
 * @param p Print interface
 */
void ESP8266WiFiClass::printDiag(Print& p)
{
	const char* modes[] =	{ "NULL", "STA", "AP", "STA+AP" };
	const char* phymodes[] =	{ "", "B", "G", "N" };
	struct station_config conf;
	char ssid[33]; //ssid can be up to 32chars, => plus null term
	char passphrase[65];

	printf("Mode: 		%s\n", modes[wifi_get_opmode()]);
	printf("PHY mode: 	%s\n", phymodes[(int) wifi_get_phy_mode()]);
	printf("Channel: 	%d\n", wifi_get_channel());
	printf("AP id: 		%d\n", wifi_station_get_current_ap_id());
	printf("Status: 	%s\n", wifi_station_get_connect_status());
	printf("Auto con: 	%d\n", wifi_station_get_auto_connect());

	wifi_station_get_config(&conf);
	memcpy(ssid, conf.ssid, sizeof(conf.ssid));
	ssid[32] = 0; 		//null term in case of 32 char ssid

	printf("SSID(%d):	%s\n", strlen(ssid), ssid);

	memcpy(passphrase, conf.password, sizeof(conf.password));
	passphrase[64] = 0;

	p.print("Pass(%d)	%s\n", strlen(passphrase), passphrase);
	p.print("BSSID set:	%s\n", conf.bssid_set);

}

ESP8266WiFiClass WiFi;
