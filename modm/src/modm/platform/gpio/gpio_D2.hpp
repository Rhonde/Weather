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

#ifndef MODM_STM32_GPIO_PIN_D2_HPP
#define MODM_STM32_GPIO_PIN_D2_HPP

#include "../device.hpp"
#include "base.hpp"
#include "set.hpp"

namespace modm
{

namespace platform
{

/// @cond
class GpioD2;
using GpioOutputD2 = GpioD2;
using GpioInputD2  = GpioD2;
/// @endcond

/// IO class for Pin D2
/// @ingroup	modm_platform_gpio
class GpioD2 : public Gpio, public ::modm::GpioIO
{
	template<class... Gpios>
	friend class GpioSet;
	using PinSet = GpioSet<GpioD2>;
	friend class Adc;
	friend class Adc1; friend class Adc2;
	friend class Adc3; friend class Adc4;
public:
	using Output = GpioD2;
	using Input = GpioD2;
	using IO = GpioD2;
	using Type = GpioD2;
	static constexpr bool isInverted = false;
	static constexpr Port port = Port::D; ///< Port name
	static constexpr uint8_t pin = 2; ///< Pin number

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
	static constexpr IRQn_Type ExternalInterruptIRQ = EXTI2_IRQn;

public:
	/// @cond
	inline static void setAlternateFunction(uint8_t af) {
		GPIOD->AFR[af_id] = (GPIOD->AFR[af_id] & ~af_mask) | ((af & 0xf) << af_offset);
		GPIOD->MODER = (GPIOD->MODER & ~mask2) | (i(Mode::AlternateFunction) << (pin * 2));
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
	inline static bool isSet() { return (GPIOD->ODR & mask); }
	// stop documentation inherited
	inline static void configure(OutputType type, OutputSpeed speed = OutputSpeed::MHz50) { PinSet::configure(type, speed); }
	inline static void setOutput(OutputType type, OutputSpeed speed = OutputSpeed::MHz50)  { PinSet::setOutput(type, speed); }
	// GpioInput
	// start documentation inherited
	inline static void setInput() { PinSet::setInput(); }
	inline static bool read() { return (GPIOD->IDR & mask); }
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
		uint32_t mode = (GPIOD->MODER & mask2);
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
		GPIOD->AFR[af_id] &= ~af_mask;
		GPIOD->ASCR &= ~mask;
	}

public:
#ifdef __DOXYGEN__
	/// @{
	/// Connect to any software peripheral
	using BitBang = GpioSignal;
	/// Connect to Sdmmc1
	using Cmd = GpioSignal;
	/// Connect to Lcd
	using Com7 = GpioSignal;
	/// Connect to Usart3
	using De = GpioSignal;
	/// Connect to Tim3
	using Etr = GpioSignal;
	/// Connect to Usart3
	using Rts = GpioSignal;
	/// Connect to Uart5
	using Rx = GpioSignal;
	/// Connect to Lcd
	using Seg31 = GpioSignal;
	/// Connect to Lcd
	using Seg43 = GpioSignal;
	/// Connect to Tsc
	using Sync = GpioSignal;
	/// @}
#endif
	/// @cond
	template< Peripheral peripheral >
	struct BitBang { static void connect();
		static_assert(
			(peripheral == Peripheral::BitBang),
			"GpioD2::BitBang only connects to software drivers!");
	};
	template< Peripheral peripheral >
	struct Cmd { static void connect();
		static_assert(
			(peripheral == Peripheral::Sdmmc1),
			"GpioD2::Cmd only connects to Sdmmc1!");
	};
	template< Peripheral peripheral >
	struct Com7 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lcd),
			"GpioD2::Com7 only connects to Lcd!");
	};
	template< Peripheral peripheral >
	struct De { static void connect();
		static_assert(
			(peripheral == Peripheral::Usart3),
			"GpioD2::De only connects to Usart3!");
	};
	template< Peripheral peripheral >
	struct Etr { static void connect();
		static_assert(
			(peripheral == Peripheral::Tim3),
			"GpioD2::Etr only connects to Tim3!");
	};
	template< Peripheral peripheral >
	struct Rts { static void connect();
		static_assert(
			(peripheral == Peripheral::Usart3),
			"GpioD2::Rts only connects to Usart3!");
	};
	template< Peripheral peripheral >
	struct Rx { static void connect();
		static_assert(
			(peripheral == Peripheral::Uart5),
			"GpioD2::Rx only connects to Uart5!");
	};
	template< Peripheral peripheral >
	struct Seg31 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lcd),
			"GpioD2::Seg31 only connects to Lcd!");
	};
	template< Peripheral peripheral >
	struct Seg43 { static void connect();
		static_assert(
			(peripheral == Peripheral::Lcd),
			"GpioD2::Seg43 only connects to Lcd!");
	};
	template< Peripheral peripheral >
	struct Sync { static void connect();
		static_assert(
			(peripheral == Peripheral::Tsc),
			"GpioD2::Sync only connects to Tsc!");
	};
	/// @endcond
private:
	template< Peripheral peripheral >
	static constexpr int8_t AdcChannel = -1;
};

/// @cond
template<>
struct GpioD2::BitBang<Peripheral::BitBang>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::BitBang;
	static constexpr int af = -1;
	inline static void connect() {}
};
template<>
struct GpioD2::Cmd<Peripheral::Sdmmc1>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Cmd;
	static constexpr int af = 12;
	inline static void
	connect()
	{
		setAlternateFunction(12);
	}
};
template<>
struct GpioD2::Com7<Peripheral::Lcd>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Com7;
	static constexpr int af = 11;
	inline static void
	connect()
	{
		setAlternateFunction(11);
	}
};
template<>
struct GpioD2::De<Peripheral::Usart3>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::De;
	static constexpr int af = 7;
	inline static void
	connect()
	{
		setAlternateFunction(7);
	}
};
template<>
struct GpioD2::Etr<Peripheral::Tim3>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Etr;
	static constexpr int af = 2;
	inline static void
	connect()
	{
		setAlternateFunction(2);
	}
};
template<>
struct GpioD2::Rts<Peripheral::Usart3>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Rts;
	static constexpr int af = 7;
	inline static void
	connect()
	{
		setAlternateFunction(7);
	}
};
template<>
struct GpioD2::Rx<Peripheral::Uart5>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Rx;
	static constexpr int af = 8;
	inline static void
	connect()
	{
		setAlternateFunction(8);
	}
};
template<>
struct GpioD2::Seg31<Peripheral::Lcd>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Seg31;
	static constexpr int af = 11;
	inline static void
	connect()
	{
		setAlternateFunction(11);
	}
};
template<>
struct GpioD2::Seg43<Peripheral::Lcd>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Seg43;
	static constexpr int af = 11;
	inline static void
	connect()
	{
		setAlternateFunction(11);
	}
};
template<>
struct GpioD2::Sync<Peripheral::Tsc>
{
	using Gpio = GpioD2;
	static constexpr Gpio::Signal Signal = Gpio::Signal::Sync;
	static constexpr int af = 9;
	inline static void
	connect()
	{
		setAlternateFunction(9);
	}
};
/// @endcond

} // namespace platform

} // namespace modm

#endif // MODM_STM32_GPIO_PIN_D2_HPP