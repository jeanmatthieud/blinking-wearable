#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define clr_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define PIN_LED_RED   1
#define PIN_LED_GREEN 3
#define PIN_LED_BLUE  2
#define PIN_SWITCH    0

// After this threshold, the red LED will blink instead of the green LED
#define THRESHOLD_COUNTER_VALUE 1800UL // 2h
// After this value, the device will sleep waiting for interrupt
#define MAX_COUNTER_VALUE (THRESHOLD_COUNTER_VALUE + 1800UL) // + 2h

volatile unsigned int g_counter = 0;

void sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
}

int main(void)
{
  // Disable ADC
  clr_bit(ADCSRA, ADEN);

  // LED pins as output
  set_bit(DDRB, PIN_LED_RED);
  set_bit(DDRB, PIN_LED_GREEN);
  // Pin switch as input
  clr_bit(DDRB, PIN_SWITCH);

  // Pull-up on switch pin
  set_bit(PORTB, PIN_SWITCH);

  // Enables PCIE interrupts on switch
  set_bit(GIMSK, PCIE);
  set_bit(PCMSK, PIN_SWITCH);

  // Change watchdog timer at 4s
  set_bit(WDTCR, WDCE);
  set_bit(WDTCR, WDP3);
  clr_bit(WDTCR, WDP2);
  clr_bit(WDTCR, WDP1);
  clr_bit(WDTCR, WDP0);

  // Enable watchdog
  clr_bit(WDTCR, WDE);
  set_bit(WDTCR, WDTIE);
  set_bit(SREG, SREG_I);

  while (1) {
    if(g_counter == 0) {
      // Acquitement
      set_bit(PORTB, PIN_LED_BLUE);
      _delay_ms(3000);
      clr_bit(PORTB, PIN_LED_BLUE);
      // Enable watchdog
      set_bit(WDTCR, WDTIE);
    } else if(g_counter >= MAX_COUNTER_VALUE) {
      // Stops watchdog
      clr_bit(WDTCR, WDTIE);
    } else {
      uint8_t ledPin = PIN_LED_GREEN;
      if(g_counter > THRESHOLD_COUNTER_VALUE) {
        ledPin = PIN_LED_RED;
      }

      set_bit(PORTB, ledPin);
      _delay_ms(100);
      clr_bit(PORTB, ledPin);
    }

    // sleep until watchdog or switch interrupt
    sleep();
  }

  return 0;
}

ISR(PCINT0_vect) {
  g_counter = 0;
}

ISR(WDT_vect) {
  g_counter++;
}
