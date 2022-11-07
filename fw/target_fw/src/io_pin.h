#pragma once

#include <avr/io.h>

template <char PORT, uint8_t PIN>
class PinRef {
public:
	static constexpr auto port = PORT | 0x20;
	static constexpr auto pin  = PIN;

	static_assert(pin >= 0 && pin <= 7, "Pin must be in the range 0-7");

	PinRef() = delete;
};

template <typename PIN_REF, bool OUTPUT, bool DEFAULT = false>
class IOPin {
public:
	constexpr IOPin() {
		if constexpr (OUTPUT) {
			write(DEFAULT);
			ddr() |= mask;
		} else {
			ddr() &= inv_mask;
		}
	}

	~IOPin() { ddr() &= inv_mask; }

	constexpr void write(const bool val) const { val ? set() : clear(); }
	constexpr void set() const { port() |= mask; }
	constexpr void clear() const { port() &= inv_mask; }
	constexpr void toggle() const { port() ^= mask; }
	constexpr bool read() const { return pin() & mask; }

private:
	static constexpr uint8_t mask     = 1 << PIN_REF::pin;
	static constexpr uint8_t inv_mask = static_cast<uint8_t>(~mask);

	template <typename _>
	struct FalseType {
		static constexpr bool val = false;
	};

	static constexpr volatile uint8_t& port() {
		if constexpr (false) {
		}
#ifdef PORTA
		else if constexpr (PIN_REF::port == 'a') {
			return PORTA;
		}
#endif
#ifdef PORTB
		else if constexpr (PIN_REF::port == 'b') {
			return PORTB;
		}
#endif
#ifdef PORTC
		else if constexpr (PIN_REF::port == 'c') {
			return PORTC;
		}
#endif
#ifdef PORTD
		else if constexpr (PIN_REF::port == 'd') {
			return PORTD;
		}
#endif
		else {
			static_assert(FalseType<void>::val, "Invalid port letter");
		}
	}

	static constexpr volatile uint8_t& pin() {
		if constexpr (false) {
		}
#ifdef PINA
		else if constexpr (PIN_REF::port == 'a') {
			return PINA;
		}
#endif
#ifdef PINB
		else if constexpr (PIN_REF::port == 'b') {
			return PINB;
		}
#endif
#ifdef PINC
		else if constexpr (PIN_REF::port == 'c') {
			return PINC;
		}
#endif
#ifdef PIND
		else if constexpr (PIN_REF::port == 'd') {
			return PIND;
		}
#endif
		else {
			static_assert(FalseType<void>::val, "Invalid port letter");
		}
	}

	static constexpr volatile uint8_t& ddr() {
		if constexpr (false) {
		}
#ifdef DDRA
		else if constexpr (PIN_REF::port == 'a') {
			return DDRA;
		}
#endif
#ifdef DDRB
		else if constexpr (PIN_REF::port == 'b') {
			return DDRB;
		}
#endif
#ifdef DDRC
		else if constexpr (PIN_REF::port == 'c') {
			return DDRC;
		}
#endif
#ifdef DDRD
		else if constexpr (PIN_REF::port == 'd') {
			return DDRD;
		}
#endif
		else {
			static_assert(FalseType<void>::val, "Invalid port letter");
		}
	}
};
