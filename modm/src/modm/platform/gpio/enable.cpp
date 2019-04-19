/*
 * Copyright (c) 2013-2014, Kevin Läufer
 * Copyright (c) 2013-2018, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "../device.hpp"

void
modm_gpio_enable(void)
{
	// Enable GPIO clock
	RCC->AHB2ENR  |=
		RCC_AHB2ENR_GPIOAEN |
		RCC_AHB2ENR_GPIOBEN |
		RCC_AHB2ENR_GPIOCEN |
		RCC_AHB2ENR_GPIODEN |
		RCC_AHB2ENR_GPIOHEN;
	// Reset GPIO peripheral
	RCC->AHB2RSTR |=
		RCC_AHB2RSTR_GPIOARST |
		RCC_AHB2RSTR_GPIOBRST |
		RCC_AHB2RSTR_GPIOCRST |
		RCC_AHB2RSTR_GPIODRST |
		RCC_AHB2RSTR_GPIOHRST;
	RCC->AHB2RSTR &= ~(
		RCC_AHB2RSTR_GPIOARST |
		RCC_AHB2RSTR_GPIOBRST |
		RCC_AHB2RSTR_GPIOCRST |
		RCC_AHB2RSTR_GPIODRST |
		RCC_AHB2RSTR_GPIOHRST);
}


// .A000100 postfix since .hardware_init is sorted alphabetically and
// this should be executed before application inits. Totally not a hack.
modm_section(".hardware_init.A000100_modm_gpio_enable")
uint32_t modm_gpio_enable_ptr = (uint32_t) &modm_gpio_enable;