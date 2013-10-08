void timer_init(void)
{
	// enable timer1 for clock cycle

	// we want at exactly 1Hz interrupt cycle
	// so we set a prescaling divider of 1024 and run the counter with a freqency of 3906.25 Hz
	// to get down to 1 Hz fro mthere we would need a divider of 65,10416666666... which is obviously not simple to set
	// so what we'll to is set the counter to 65 and on each 1/10 run we'll set the divider to 66
	//  actually we'll see if this is necessary for our 4 1/3 minute countdown...

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// prescaler to 1024
	SETBITS(TCCR1B, BIT(CS12) | BIT(CS10));

	// auto-clear the counter on output-compare match (timer 0)
	SETBIT(TCCR1B, WGM12);

	// set output-compare-value to 6 (5 counts) on (timer 0, compare A)
	OCR1A = 3906;

	// enable output-compare interrupt (timer 0, compare A)
	SETBIT(TIMSK, OCIE1A);

	// restore system state
	SREG = sreg_tmp;
}


// the maximum time displayable is 99 Minutes and 59 Seconds, which equals 5999 Seconds. Therefore we need an 16 bit counter variable
volatile uint16_t remaining_time = 0;

void set_remaining_time(uint16_t time)
{
	// guard
	if(time > 5999)
		return;

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();


	remaining_time = time;
	display_set_time(time);

	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;	
}

ISR(TIMER1_COMPA_vect)
{
	if(remaining_time == 0)
	{
		display_set_done();
		return;
	}

	remaining_time--;
	display_set_time(remaining_time);
}
