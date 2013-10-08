#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <bits.h>
#include <pinout.h>

#include <display.c>
#include <mosfet.c>
#include <timer.c>
#include <buttons.c>

int __attribute__((OS_main))
main(void)
{
	display_init();
	buttons_init();
	mosfet_init();
	
	timer_set_remaining(60);
	timer_init();

	// enable interrupts
	sei();

	while(1);
	return 0;
}
