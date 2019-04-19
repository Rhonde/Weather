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

#ifndef MODM_STM32_GPIO_PIN_B7_HPP
#define MODM_STM32_GPIO_PIN_B7_HPP

#include "../device.hpp"
#include "base.hpp"
#include "set.hpp"

namespace modm
{

namespace platform
{

/// @cond
class GpioB7;
using GpioOutputB7 = GpioB7;
using GpioInputB7  = GpioB7;
/// @endcond

/// IO class for Pin B7
/// @ingroup	modm_platform_gpio
class GpioB7 : public Gpio, public ::modm::GpioIO
{
	template<class... Gpios>
	friend class GpioSet;
	using PinSet = GpioSet<GpioB7>;
	friend class Adc;
	friend class Adc1; friend class Adc2;
	friend class Adc3; friend class Adc4;
public:
	using Output = GpioB7;
	using Input = GpioB7;
	using IO = GpioB7;
	using Type = GpioB7;
	static constexpr bool isInverted = false;
	static constexpr Port port = Port::B; ///< Port name
	static constexpr uint8_t pin = 7; ///< Pin number

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
	static constexpr IRQn_Type ExternalInterruptIRQ = EXTI9_5_IRQn;

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
	/// Connect to Tim8
	using Bkin = GpioSignal;
	/// Connect to Tim8
	using BkinComp1 = GpioSignal;
	/// Connect to Tim17
	using Ch1n = GpioSignal;
	/// Connect to Tim4
	using Ch2 = GpioSignal;
	/// Connect to Dfsdm1
	using Ckin5 = GpioSignal;
	/// Connect to Uart4
	using Cts = GpioSignal;
	/// Connect to Tsc
	using G2Io4 = GpioSignal;
	/// Connect to Lptim1
	using In2 = GpioSignal;
	/// Connect to Comp2
	using Inm = GpioSignal;
	/// Connect to Sys
	using PvdIn = GpioSignal;
	/// Connect to Usart1
	using Rx = GpioSignal;
	/// Connect to I2c1
	using Sda = GpioSignal;
	/// Connect to Lcd
	using Seg21 = GpioSignal;
	/// @}
#endif
	/// @cond
	template< Peripheral peripheral >
	struct BitBang { static void connect();
		static_assert(
			(peripheral == Peripheral::BitBang),
			"GpioB7::BitBang only connects to software drivers!");
	};
	template< Peripheral peripheral >
	struct Bkin { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim8),
			"GpioB7::Bkin only connects to Tim8!");
	};
	template< Peripheral peripheral >
	struct BkinComp1 { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim8),
			"GpioB7::BkinComp1 only connects to Tim8!");
	};
	template< Peripheral peripheral >
	struct Ch1n { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim17),
			"GpioB7::Ch1n only connects to Tim17!");
	};
	template< Peripheral peripheral >
	struct Ch2 { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim4),
			"GpioB7::Ch2 only connects to Tim4!");
	};
	template< Peripheral peripheral >
	struct Ckin5 { static void connect();
		static_assert(
			(peripheral == Peripheral::Dfsdm1),
			"GpioB7::Ckin5 only connects to Dfsdm1!");
	};
	template< Peripheral peripheral >
	struct Cts { static void connect();
		static_assert(
			(peripheral == Peripheral::Uart4),
			"GpioB7::Cts only connects to Uart4!");
	};
	template< Peripheral peripheral >
	struct G2Io4 { static void connect();
		static_assert(
			(peripheral == Peripheral::Tsc),
			"GpioB7::G2Io4 only connects to Tsc!");
	};
	template< Peripheral peripheral >
	struct In2 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lptim1),
			"GpioB7::In2 only connects to Lptim1!");
	};
	template< Peripheral peripheral >
	struct Inm { static void connect();
		static_assert(
			(peripheral == Peripheral::Comp2),
			"GpioB7::Inm only connects to Comp2!");
	};
	template< Peripheral peripheral >
	struct PvdIn { static void connect();
		static_assert(
			(peripheral == Peripheral::Sys),
			"GpioB7::PvdIn only connects to Sys!");
	};
	template< Peripheral peripheral >
	struct Rx { static void connect();
		static_assert(
			(peripheral == Peripheral::Usart1),
			"GpioB7::Rx only connects to Usart1!");
	};
	template< Peripheral peripheral >
	struct Sda { static void connect();
		static_assert(
			(peripheral == Peripheral::I2c1),
			"GpioB7::Sda only connects to I2c1!");
	};
	template< Peripheral peripheral >
	struct Seg21 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lcd),
			"GpioB7::Seg21 only connects to Lcd!");
	};
	/// @endcond
private:
	template< Peripheral peripheral >
	static constexpr int8_t AdcChannel = -1;
};

/// @cond
template<>
struct GpioB7::BitBang<Peripheral::BitBang>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::BitBang;
	static constexpr int af = -1;
	inline static void connect() {}
};
template<>
struct GpioB7::Bkin<Peripheral::Tim8>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Bkin;
	static constexpr int af = 3;
	inline static void
	connect()
	{
		setAlternateFunction(3);
	}
};
template<>
struct GpioB7::BkinComp1<Peripheral::Tim8>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::BkinComp1;
	static constexpr int af = 13;
	inline static void
	connect()
	{
		setAlternateFunction(13);
	}
};
template<>
struct GpioB7::Ch1n<Peripheral::Tim17>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ch1n;
	static constexpr int af = 14;
	inline static void
	connect()
	{
		setAlternateFunction(14);
	}
};
template<>
struct GpioB7::Ch2<Peripheral::Tim4>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ch2;
	static constexpr int af = 2;
	inline static void
	connect()
	{
		setAlternateFunction(2);
	}
};
template<>
struct GpioB7::Ckin5<Peripheral::Dfsdm1>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Ckin5;
	static constexpr int af = 6;
	inline static void
	connect()
	{
		setAlternateFunction(6);
	}
};
template<>
struct GpioB7::Cts<Peripheral::Uart4>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Cts;
	static constexpr int af = 8;
	inline static void
	connect()
	{
		setAlternateFunction(8);
	}
};
template<>
struct GpioB7::G2Io4<Peripheral::Tsc>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::G2Io4;
	static constexpr int af = 9;
	inline static void
	connect()
	{
		setAlternateFunction(9);
	}
};
template<>
struct GpioB7::In2<Peripheral::Lptim1>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::In2;
	static constexpr int af = 1;
	inline static void
	connect()
	{
		setAlternateFunction(1);
	}
};
template<>
struct GpioB7::Inm<Peripheral::Comp2>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Inm;
	static constexpr int af = -1;
	inline static void
	connect()
	{
		disconnect();
		setAnalogInput();
	}
};
template<>
struct GpioB7::PvdIn<Peripheral::Sys>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::PvdIn;
	static constexpr int af = -1;
	inline static void
	connect()
	{
	}
};
template<>
struct GpioB7::Rx<Peripheral::Usart1>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Rx;
	static constexpr int af = 7;
	inline static void
	connect()
	{
		setAlternateFunction(7);
	}
};
template<>
struct GpioB7::Sda<Peripheral::I2c1>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Sda;
	static constexpr int af = 4;
	inline static void
	connect()
	{
		setAlternateFunction(4);
	}
};
template<>
struct GpioB7::Seg21<Peripheral::Lcd>
{
	using Gpio = GpioB7;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Seg21;
	static constexpr int af = 11;
	inline static void
	connect()
	{
		setAlternateFunction(11);
	}
};
/// @endcond

} // namespace platform

} // namespace modm

#endif // MODM_STM32_GPIO_PIN_B7_HPP