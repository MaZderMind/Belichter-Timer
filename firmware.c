#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <bits.h>
#include <pinout.h>

#include <display.c>
#include <timer.c>
#include <buttons.c>

int __attribute__((OS_main))
main(void)
{
	display_init();
	buttons_init();
	
	set_remaining_time(60);
	timer_init();

	// enable interrupts
	sei();

	SETBIT(DDRA, PA1);

	while(1)
	{
		TOGGLEBIT(PORTA, PA1);
		_delay_ms(1000);
	}

	return 0;
}
