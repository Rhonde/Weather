/*
 * Copyright (c) 2016-2018, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_GPIO_BASE_HPP
#define MODM_STM32_GPIO_BASE_HPP

#include "../device.hpp"
#include <modm/architecture/interface/gpio.hpp>
#include <modm/math/utils/bit_operation.hpp>
#include <modm/platform/core/peripherals.hpp>

namespace modm
{

namespace platform
{

/// @ingroup modm_platform_gpio
struct Gpio
{
	enum class
	InputType
	{
		Floating = 0x0,	///< floating on input
		PullUp = 0x1,	///< pull-up on input
		PullDown = 0x2,	///< pull-down on input
	};

	enum class
	OutputType
	{
		PushPull = 0x0,		///< push-pull on output
		OpenDrain = 0x1,	///< open-drain on output
	};

	enum class
	OutputSpeed
	{
		Low      = 0,
		Medium   = 0x1,
		High     = 0x2,
		VeryHigh = 0x3,		///< 30 pF (80 MHz Output max speed on 15 pF)
		MHz2     = Low,
		MHz25    = Medium,
		MHz50    = High,
		MHz100   = VeryHigh,
	};

	enum class
	InputTrigger
	{
		RisingEdge,
		FallingEdge,
		BothEdges,
	};

	/// The Port a Gpio Pin is connected to.
	enum class
	Port
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3,
		H = 7,
	};

	/// @cond
	enum class
	Signal
	{
		BitBang,
		Bk1Io0,
		Bk1Io1,
		Bk1Io2,
		Bk1Io3,
		Bkin,
		Bkin2,
		Bkin2Comp1,
		Bkin2Comp2,
		BkinComp1,
		BkinComp2,
		Ch1,
		Ch1n,
		Ch2,
		Ch2n,
		Ch3,
		Ch3n,
		Ch4,
		Ck,
		Ckin0,
		Ckin1,
		Ckin2,
		Ckin3,
		Ckin4,
		Ckin5,
		Ckin6,
		Ckin7,
		Ckout,
		Clk,
		Cmd,
		Com0,
		Com1,
		Com2,
		Com3,
		Com4,
		Com5,
		Com6,
		Com7,
		Cts,
		D0,
		D1,
		D2,
		D3,
		D4,
		D5,
		D6,
		D7,
		Datin0,
		Datin1,
		Datin2,
		Datin3,
		Datin4,
		Datin5,
		Datin6,
		Datin7,
		De,
		Dm,
		Dp,
		Etr,
		Extclk,
		FsA,
		FsB,
		G1Io1,
		G1Io2,
		G1Io3,
		G1Io4,
		G2Io1,
		G2Io2,
		G2Io3,
		G2Io4,
		G3Io1,
		G3Io2,
		G3Io3,
		G3Io4,
		G4Io1,
		G4Io2,
		G4Io3,
		G4Io4,
		Id,
		In1,
		In10,
		In11,
		In12,
		In13,
		In14,
		In15,
		In16,
		In2,
		In3,
		In4,
		In5,
		In6,
		In7,
		In8,
		In9,
		Inm,
		Inp,
		Io,
		IrOut,
		Jtck,
		Jtdi,
		Jtdo,
		Jtms,
		Jtrst,
		Lsco,
		MclkA,
		MclkB,
		Mco,
		Miso,
		Mosi,
		Ncs,
		Noe,
		Nss,
		Osc32In,
		Osc32Out,
		OscIn,
		OscOut,
		Out,
		Out1,
		Out2,
		OutAlarm,
		OutCalib,
		PvdIn,
		Refin,
		Rts,
		Rx,
		Sck,
		SckA,
		SckB,
		Scl,
		SdA,
		SdB,
		Sda,
		Seg0,
		Seg1,
		Seg10,
		Seg11,
		Seg12,
		Seg13,
		Seg14,
		Seg15,
		Seg16,
		Seg17,
		Seg18,
		Seg19,
		Seg2,
		Seg20,
		Seg21,
		Seg22,
		Seg23,
		Seg24,
		Seg25,
		Seg26,
		Seg27,
		Seg28,
		Seg29,
		Seg3,
		Seg30,
		Seg31,
		Seg4,
		Seg40,
		Seg41,
		Seg42,
		Seg43,
		Seg5,
		Seg6,
		Seg7,
		Seg8,
		Seg9,
		Smba,
		Sof,
		Suspend,
		Swclk,
		Swdio,
		Swo,
		Sync,
		Tamp1,
		Tamp2,
		Ts,
		Tx,
		Vbus,
		Vinm,
		Vinp,
		Vlcd,
		Vout,
		Wkup1,
		Wkup2,
		Wkup4,
		Wkup5,
	};
	/// @endcond

protected:
	/// @cond
	/// I/O Direction Mode values for this specific pin.
	enum class
	Mode
	{
		Input  = 0x0,
		Output = 0x1,
		AlternateFunction = 0x2,
		Analog = 0x3,
		Mask   = 0x3,
	};

	static constexpr uint32_t
	i(Mode mode) { return uint32_t(mode); }
	// Enum Class To Integer helper functions.
	static constexpr uint32_t
	i(InputType pull) { return uint32_t(pull); }
	static constexpr uint32_t
	i(OutputType out) { return uint32_t(out); }
	static constexpr uint32_t
	i(OutputSpeed speed) { return uint32_t(speed); }
	/// @endcond
};

} // namespace platform

} // namespace modm

#endif // MODM_STM32_GPIO_BASE_HPP