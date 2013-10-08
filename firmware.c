#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <bits.h>
#include <pinout.h>
#include <display.c>

int __attribute__((OS_main))
main(void)
{
	display_init();

	SETBIT(DDRA, PA1);

	uint16_t cnt = 0;
	while(1)
	{
		//TOGGLEBIT(PORTA, PA1);
		_delay_ms(100);
		cnt++;

		if(cnt > 110)
		{
			display_set_done();
		}
		else if(cnt > 100)
		{
			display_set_open();
			_delay_ms(500);
			display_set_empty();
			_delay_ms(500);
		}
		else
		{
			display_set_number(cnt);
		}
	}

	return 0;
}
