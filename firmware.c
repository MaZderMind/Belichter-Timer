#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <bits.h>

int __attribute__((OS_main))
main(void)
{
	SETBIT(DDRA, PA1);

	while(1)
	{
		SETBIT(PORTA, PA1);
		_delay_ms(500);
		CLEARBIT(PORTA, PA1);
		_delay_ms(500);


		//SETBIT(PORTB, PB7);
		//_delay_ms(100);
		//CLEARBIT(PORTB, PB7);
		//_delay_ms(100);
	}

	return 0;
}