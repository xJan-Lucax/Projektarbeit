#pragma once

#define PIN_OUTPUT(port, pin) SET_BIT((port), (pin))
#define PIN_INPUT(port, pin) CLEAR_BIT((port), (pin))

#define SET_BIT(register, bit) (register) |= (1 << (bit))
#define CLEAR_BIT(register, bit) (register) &= ~(1 << (bit))

#define IS_PIN_HIGH(port, pin) ((port) & (1 << (pin)))
#define IS_PIN_LOW(port, pin) (!IS_PIN_HIGH((port), (pin)))
