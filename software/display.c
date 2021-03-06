inline void display_init(void)
{
	// enable timer0 for display update cycle

	// we want at least 100Hz update rate on the display
	//  to redraw all segments at a rate of 100 Hz we need a 100 Hz * 7 segments * 4 digits = 2.8 kHz timer clock
	// with the chip running at 4 MHz this is 1/1428 of the clock, so we want a clock divider as close to 1428 as possible
	// so we set a prescaling divider of 256 and count from 0 to 4. when we reach 4 we'll trigger the interrupt routine
	// that gives us an interrupt frequency of 3.125 kHz per-segment redraw cycle of 111.6 Hz at a duty rate of 3.125%

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// prescaler to 256
	SETBIT(TCCR0B, CS02);

	// auto-clear the counter on output-compare match (timer 0)
	SETBIT(TCCR0A, WGM01);

	// set output-compare-value to 4 (5 counts) on (timer 0, compare A)
	OCR0A = 4;

	// enable output-compare interrupt (timer 0, compare A)
	SETBIT(TIMSK, OCIE0A);


	// configure anodes (aka digits)
	SETBIT(DDR_A1, P_A1);
	SETBIT(DDR_A2, P_A2);
	SETBIT(DDR_A3, P_A3);
	SETBIT(DDR_A4, P_A4);

	// disable all anodes (aka digits)
	CLEARBIT(PORT_A1, P_A1);
	CLEARBIT(PORT_A2, P_A2);
	CLEARBIT(PORT_A3, P_A3);
	CLEARBIT(PORT_A4, P_A4);

	// configure cathodes (aka segments)
	SETBIT(DDR_CA, P_CA);
	SETBIT(DDR_CB, P_CB);
	SETBIT(DDR_CC, P_CC);
	SETBIT(DDR_CD, P_CD);
	SETBIT(DDR_CE, P_CE);
	SETBIT(DDR_CF, P_CF);
	SETBIT(DDR_CG, P_CG);

	// disable all cathodes (aka segments)
	SETBIT(PORT_CA, P_CA);
	SETBIT(PORT_CB, P_CB);
	SETBIT(PORT_CC, P_CC);
	SETBIT(PORT_CD, P_CD);
	SETBIT(PORT_CE, P_CE);
	SETBIT(PORT_CF, P_CF);
	SETBIT(PORT_CG, P_CG);

	// restore system state
	SREG = sreg_tmp;
}

uint8_t cgen[] = {
	/*       GFEDCBA*/
	/* 0 */0b0111111,
	/* 1 */0b0110000,
	/* 2 */0b1011011,
	/* 3 */0b1111001,
	/* 4 */0b1110100,
	/* 5 */0b1101101,
	/* 6 */0b1101111,
	/* 7 */0b0111000,
	/* 8 */0b1111111,
	/* 9 */0b1111101,
	/*       GFEDCBA*/

	/* 10 => '' */0b000000,

	/*             GFEDCBA*/
	/* 11 => P */0b1011110,
	/* 12 => E */0b1001111,
	/* 13 => N */0b0111110,
	/* 14 => d */0b1110011,
	/* 15 => t */0b1000111,
	/*             GFEDCBA*/
};

volatile uint8_t digits[4];
void display_set_number(uint16_t number)
{
	// guard
//	if(number > 9999)
//		return;

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// update display-number in-memory (volatile)
	for(uint8_t i = 0; i < 4; i++)
	{
		if(number > 0)
		{
			digits[i] = number % 10;
			number /= 10;
		}
		else digits[i] = 0;
	}

	// restore system state
	SREG = sreg_tmp;
}

// the maximum time displayable is 99 Minutes and 59 Seconds, which equals 5999 Seconds
#define TWO_DIGIT_MINUTES 1

void display_set_time(uint16_t number)
{
	// guard
//	if(number > 5999)
//		return;

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// update display-number in-memory (volatile)
	if(number > 0)
	{
		digits[0] = number % 10;
		number /= 10;
	}
	else digits[0] = 0;


	if(number > 0)
	{
		digits[1] = number % 6;
		number /= 6;
	}
	else digits[1] = 0;


#if TWO_DIGIT_MINUTES

	if(number > 0)
	{
		digits[2] = number % 10;
		number /= 10;
	}
	else digits[2] = 0;

	if(number > 0)
	{
		digits[3] = number % 10;
		number /= 10;
	}
	else digits[3] = 10;

#else

	digits[2] = 10;

	if(number > 0)
	{
		digits[3] = number % 10;
		number /= 10;
	}
	else digits[3] = 0;

#endif

	// restore system state
	SREG = sreg_tmp;
}

void display_set_open(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	digits[0] = 13;
	digits[1] = 12;
	digits[2] = 11;
	digits[3] = 0;

	// restore system state
	SREG = sreg_tmp;
}

void display_set_done(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	digits[0] = 12;
	digits[1] = 13;
	digits[2] = 0;
	digits[3] = 14;

	// restore system state
	SREG = sreg_tmp;
}

void display_set_empty(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	digits[0] = 10;
	digits[1] = 10;
	digits[2] = 10;
	digits[3] = 10;

	// restore system state
	SREG = sreg_tmp;
}

void display_set_set(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	digits[0] = 10;
	digits[1] = 15;
	digits[2] = 12;
	digits[3] = 5;

	// restore system state
	SREG = sreg_tmp;
}

// just for the records:
//   i tried to implement this with lookup tables but it is actually more code,
//   because the lookuptable goes into the ram and ram lookups need extra ops
//     this implementation works with just a bunch (actually a whole bunch) of
//   registets and a set of cpi, brne, sbi and rjmp instructions
ISR(TIMER0_COMPA_vect)
{
	// branch into buttons.c for button debouncing
	button_debounce_isr();

	static uint8_t segment_cnt = 0;
	static uint8_t segment = 0; 
	static uint8_t digit = 0;

	// disable last-active anode (aka digit)
	if(digit == 3)
		CLEARBIT(PORT_A1, P_A1);
	else if(digit == 2)
		CLEARBIT(PORT_A2, P_A2);
	else if(digit == 1)
		CLEARBIT(PORT_A3, P_A3);
	else if(digit == 0)
		CLEARBIT(PORT_A4, P_A4);

	// disable last active cathode (aka segment)
	if(segment == 0)
		SETBIT(PORT_CA, P_CA);
	else if(segment == 1)
		SETBIT(PORT_CB, P_CB);
	else if(segment == 2)
		SETBIT(PORT_CC, P_CC);
	else if(segment == 3)
		SETBIT(PORT_CD, P_CD);
	else if(segment == 4)
		SETBIT(PORT_CE, P_CE);
	else if(segment == 5)
		SETBIT(PORT_CF, P_CF);
	else if(segment == 6)
		SETBIT(PORT_CG, P_CG);

	// increment counter w/ wrap around
	if(++segment_cnt == 7 * 4) segment_cnt = 0;

	// calculate next digit and segment
	segment = segment_cnt % 7; 
	digit = segment_cnt / 7;

	uint8_t digit_to_display = cgen[digits[3 - digit]];
	if(BITSET(digit_to_display, segment))
	{
		// enable anode (aka digit)
		if(digit == 3)
			SETBIT(PORT_A1, P_A1);
		else if(digit == 2)
			SETBIT(PORT_A2, P_A2);
		else if(digit == 1)
			SETBIT(PORT_A3, P_A3);
		else if(digit == 0)
			SETBIT(PORT_A4, P_A4);

		// enable cathode (aka segment)
		if(segment == 0)
			CLEARBIT(PORT_CA, P_CA);
		else if(segment == 1)
			CLEARBIT(PORT_CB, P_CB);
		else if(segment == 2)
			CLEARBIT(PORT_CC, P_CC);
		else if(segment == 3)
			CLEARBIT(PORT_CD, P_CD);
		else if(segment == 4)
			CLEARBIT(PORT_CE, P_CE);
		else if(segment == 5)
			CLEARBIT(PORT_CF, P_CF);
		else if(segment == 6)
			CLEARBIT(PORT_CG, P_CG);
	}
}
