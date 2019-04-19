/*
 * Copyright (c) 2017-2018, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_GPIO_PIN_B15_HPP
#define MODM_STM32_GPIO_PIN_B15_HPP

#include "../device.hpp"
#include "base.hpp"
#include "set.hpp"

namespace modm
{

namespace platform
{

/// @cond
class GpioB15;
using GpioOutputB15 = GpioB15;
using GpioInputB15  = GpioB15;
/// @endcond

/// IO class for Pin B15
/// @ingroup	modm_platform_gpio
class GpioB15 : public Gpio, public ::modm::GpioIO
{
	template<class... Gpios>
	friend class GpioSet;
	using PinSet = GpioSet<GpioB15>;
	friend class Adc;
	friend class Adc1; friend class Adc2;
	friend class Adc3; friend class Adc4;
public:
	using Output = GpioB15;
	using Input = GpioB15;
	using IO = GpioB15;
	using Type = GpioB15;
	static constexpr bool isInverted = false;
	static constexpr Port port = Port::B; ///< Port name
	static constexpr uint8_t pin = 15; ///< Pin number

protected:
	/// Bitmask for registers that contain a 1bit value for every pin.
	static constexpr uint16_t mask  = 0x1 << pin;
	/// Bitmask for registers that contain a 2bit value for every pin.
	static constexpr uint32_t mask2 = 0x3 << (pin * 2);
	/// Port Number.
	static constexpr uint8_t port_nr = uint8_t(port);
	/// Alternate Function register id. 0 for pin 0-7. 1 for pin 8-15.
	static constexpr uint8_t af_id  = pin / 8;
	/// Alternate Function offset.
	static constexpr uint8_t af_offset = (pin * 4) % 32;
	/// Alternate Function register mask.
	static constexpr uint32_t af_mask  = 0xf << af_offset;
	/// ExternalInterruptIRQ
	static constexpr IRQn_Type ExternalInterruptIRQ = EXTI15_10_IRQn;

public:
	/// @cond
	inline static void setAlternateFunction(uint8_t af) {
		GPIOB->AFR[af_id] = (GPIOB->AFR[af_id] & ~af_mask) | ((af & 0xf) << af_offset);
		GPIOB->MODER = (GPIOB->MODER & ~mask2) | (i(Mode::AlternateFunction) << (pin * 2));
	}

	/// Enable Analog Mode which is needed to use this pin as an ADC input.
	inline static void setAnalogInput() { PinSet::setAnalogInput(); }
	/// @endcond

public:
	// GpioOutput
	// start documentation inherited
	inline static void setOutput() { PinSet::setOutput(); }
	inline static void setOutput(bool status) { PinSet::setOutput(status); }
	inline static void set() { PinSet::set(); }
	inline static void set(bool status) { PinSet::set(status); }
	inline static void reset() { PinSet::reset(); }
	inline static void toggle() {
		if (isSet()) reset();
		else         set();
	}
	inline static bool isSet() { return (GPIOB->ODR & mask); }
	// stop documentation inherited
	inline static void configure(OutputType type, OutputSpeed speed = OutputSpeed::MHz50) { PinSet::configure(type, speed); }
	inline static void setOutput(OutputType type, OutputSpeed speed = OutputSpeed::MHz50)  { PinSet::setOutput(type, speed); }
	// GpioInput
	// start documentation inherited
	inline static void setInput() { PinSet::setInput(); }
	inline static bool read() { return (GPIOB->IDR & mask); }
	// end documentation inherited
	inline static void configure(InputType type) { PinSet::configure(type); }
	inline static void setInput(InputType type) { PinSet::setInput(type); }
	// External Interrupts
	// Warning: This will disable any previously enabled interrupt which is
	// routed to the same interupt line, e.g. PA3 will disable PB3.
	// This is a hardware limitation by the STM32 EXTI.
	inline static void enableExternalInterrupt()
	{
		// PA[x], x =  0 ..  3 maps to EXTICR[0]
		// PA[x], x =  4 ..  7 maps to EXTICR[1]
		// PA[x], x =  8 .. 11 maps to EXTICR[2]
		// PA[x], x = 12 .. 15 maps to EXTICR[3]
		// => bit3 and bit2 (mask 0x0c) specify the register
		// => bit1 and bit0 (mask 0x03) specify the bit position
		constexpr uint8_t index   = (pin & 0b1100) >> 2;
		constexpr uint8_t bit_pos = (pin & 0b0011) << 2;
		constexpr uint16_t syscfg_mask = (0b1111) << bit_pos;
		constexpr uint16_t syscfg_value = (port_nr & (0b1111)) << bit_pos;
		SYSCFG->EXTICR[index] = (SYSCFG->EXTICR[index] & ~syscfg_mask) | syscfg_value;
		EXTI->IMR1 |= mask;
	}
	inline static void disableExternalInterrupt() { EXTI->IMR1 &= ~mask; }
	inline static void enableExternalInterruptVector(const uint32_t priority)
	{
		NVIC_SetPriority(ExternalInterruptIRQ, priority);
		NVIC_EnableIRQ(ExternalInterruptIRQ);
	}
	inline static void disableExternalInterruptVector() { NVIC_DisableIRQ(ExternalInterruptIRQ); }
	inline static void setInputTrigger(const InputTrigger trigger)
	{
		switch (trigger)
		{
		case InputTrigger::RisingEdge:
			EXTI->RTSR1 |=  mask;
			EXTI->FTSR1 &= ~mask;
			break;
		case InputTrigger::FallingEdge:
			EXTI->RTSR1 &= ~mask;
			EXTI->FTSR1 |=  mask;
			break;
		case InputTrigger::BothEdges:
			EXTI->RTSR1 |=  mask;
			EXTI->FTSR1 |=  mask;
			break;
		}
	}
	inline static bool getExternalInterruptFlag() { return (EXTI->PR1 & mask); }
	inline static void acknowledgeExternalInterruptFlag() { EXTI->PR1 |= mask; }
	// GpioIO
	// start documentation inherited
	inline static Direction getDirection() {
		uint32_t mode = (GPIOB->MODER & mask2);
		if (mode == (i(Mode::Input) << pin * 2))
			return Direction::In;
		if (mode == (i(Mode::Output) << pin * 2))
			return Direction::Out;
		return Direction::Special;
	}
	// end documentation inherited
	inline static void lock() { PinSet::lock(); }
	inline static void disconnect() {
		setInput(InputType::Floating);
		GPIOB->AFR[af_id] &= ~af_mask;
		GPIOB->ASCR &= ~mask;
	}

public:
#ifdef __DOXYGEN__
	/// @{
	/// Connect to any software peripheral
	using BitBang = GpioSignal;
	/// Connect to Tim15
	using Ch2 = GpioSignal;
	/// Connect to Tim1 or Tim8
	using Ch3n = GpioSignal;
	/// Connect to Dfsdm1
	using Ckin2 = GpioSignal;
	/// Connect to Tsc
	using G1Io4 = GpioSignal;
	/// Connect to Spi2
	using Mosi = GpioSignal;
	/// Connect to Rtc
	using Refin = GpioSignal;
	/// Connect to Sai2
	using SdA = GpioSignal;
	/// Connect to Lcd
	using Seg15 = GpioSignal;
	/// Connect to Swpmi1
	using Suspend = GpioSignal;
	/// @}
#endif
	/// @cond
	template< Peripheral peripheral >
	struct BitBang { static void connect();
		static_assert(
			(peripheral == Peripheral::BitBang),
			"GpioB15::BitBang only connects to software drivers!");
	};
	template< Peripheral peripheral >
	struct Ch2 { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim15),
			"GpioB15::Ch2 only connects to Tim15!");
	};
	template< Peripheral peripheral >
	struct Ch3n { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim1) ||
			(peripheral == Peripheral::Tim8),
			"GpioB15::Ch3n only connects to Tim1 or Tim8!");
	};
	template< Peripheral peripheral >
	struct Ckin2 { static void connect();
		static_assert(
			(peripheral == Peripheral::Dfsdm1),
			"GpioB15::Ckin2 only connects to Dfsdm1!");
	};
	template< Peripheral peripheral >
	struct G1Io4 { static void connect();
		static_assert(
			(peripheral == Peripheral::Tsc),
			"GpioB15::G1Io4 only connects to Tsc!");
	};
	template< Peripheral peripheral >
	struct Mosi { static void connect();
		static_assert(
			(peripheral == Peripheral::Spi2),
			"GpioB15::Mosi only connects to Spi2!");
	};
	template< Peripheral peripheral >
	struct Refin { static void connect();
		static_assert(
			(peripheral == Peripheral::Rtc),
			"GpioB15::Refin only connects to Rtc!");
	};
	template< Peripheral peripheral >
	struct SdA { static void connect();
		static_assert(
			(peripheral == Peripheral::Sai2),
			"GpioB15::SdA only connects to Sai2!");
	};
	template< Peripheral peripheral >
	struct Seg15 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lcd),
			"GpioB15::Seg15 only connects to Lcd!");
	};
	template< Peripheral peripheral >
	struct Suspend { static void connect();
		static_assert(
			(peripheral == Peripheral::Swpmi1),
			"GpioB15::Suspend only connects to Swpmi1!");
	};
	/// @endcond
private:
	template< Peripheral peripheral >
	static constexpr int8_t AdcChannel = -1;
};

/// @cond
template<>
struct GpioB15::BitBang<Peripheral::BitBang>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::BitBang;
	static constexpr int af = -1;
	inline static void connect() {}
};
template<>
struct GpioB15::Ch2<Peripheral::Tim15>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ch2;
	static constexpr int af = 14;
	inline static void
	connect()
	{
		setAlternateFunction(14);
	}
};
template<>
struct GpioB15::Ch3n<Peripheral::Tim1>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ch3n;
	static constexpr int af = 1;
	inline static void
	connect()
	{
		setAlternateFunction(1);
	}
};
template<>
struct GpioB15::Ch3n<Peripheral::Tim8>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ch3n;
	static constexpr int af = 3;
	inline static void
	connect()
	{
		setAlternateFunction(3);
	}
};
template<>
struct GpioB15::Ckin2<Peripheral::Dfsdm1>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ckin2;
	static constexpr int af = 6;
	inline static void
	connect()
	{
		setAlternateFunction(6);
	}
};
template<>
struct GpioB15::G1Io4<Peripheral::Tsc>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::G1Io4;
	static constexpr int af = 9;
	inline static void
	connect()
	{
		setAlternateFunction(9);
	}
};
template<>
struct GpioB15::Mosi<Peripheral::Spi2>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Mosi;
	static constexpr int af = 5;
	inline static void
	connect()
	{
		setAlternateFunction(5);
	}
};
template<>
struct GpioB15::Refin<Peripheral::Rtc>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Refin;
	static constexpr int af = 0;
	inline static void
	connect()
	{
		setAlternateFunction(0);
	}
};
template<>
struct GpioB15::SdA<Peripheral::Sai2>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::SdA;
	static constexpr int af = 13;
	inline static void
	connect()
	{
		setAlternateFunction(13);
	}
};
template<>
struct GpioB15::Seg15<Peripheral::Lcd>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Seg15;
	static constexpr int af = 11;
	inline static void
	connect()
	{
		setAlternateFunction(11);
	}
};
template<>
struct GpioB15::Suspend<Peripheral::Swpmi1>
{
	using Gpio = GpioB15;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Suspend;
	static constexpr int af = 12;
	inline static void
	connect()
	{
		setAlternateFunction(12);
	}
};
/// @endcond

} // namespace platform

} // namespace modm

#endif // MODM_STM32_GPIO_PIN_B15_HPP