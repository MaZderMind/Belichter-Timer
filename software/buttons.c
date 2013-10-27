void buttons_init(void)
{
	// configure reset pin to ie an input
	SETBIT(DDR_RESET, P_RESET);

	// enable pullup for reset btn
	SETBIT(PORT_RESET, P_RESET);

	// enable pin change interrupt for reset one of the buttons
	SETBIT(PCMSK, PCINT_RESET);


	// configure start pin to ie an input
	SETBIT(DDR_START, P_START);

	// enable pullup for start btn
	SETBIT(PORT_START, P_START);

	// enable pin change interrupt for start one of the buttons
	SETBIT(PCMSK, PCINT_START);



	// enable pin change interrupts in general
	SETBIT(GIMSK, PCIE);
}


#define DEBOUNCE_HI 50
#define LONGPRESS_HI 6250 // 2s / 320µs

volatile uint8_t reset_cnt = 0, start_cnt = 0;
volatile uint16_t both_longpress_cnt = 0;

void button_debounce_isr(void)
{
	if(reset_cnt > 0)
	{
		// timer for reset is over and the start button is not pressed in parallel (not both pressed)
		if(--reset_cnt == 0 && BITSET(PIN_START, P_START))
			timer_reset_requested();
	}

	if(start_cnt > 0)
	{
		// timer for reset is over and the start button is not pressed in parallel (not both pressed)
		if(--start_cnt == 0 && BITSET(PIN_RESET, P_RESET))
			timer_startstop_requested();
	}

	if(both_longpress_cnt > 0)
	{
		if(--both_longpress_cnt == 0)
			timer_setmode_requested();
	}

}

ISR(PCINT_vect)
{
	if(BITCLEAR(PIN_RESET, P_RESET))
	{
		reset_cnt = DEBOUNCE_HI;
	}
	else
	{
		reset_cnt = 0;
	}

	if(BITCLEAR(PIN_START, P_START))
	{
		start_cnt = DEBOUNCE_HI;
	}
	else
	{
		start_cnt = 0;
	}

	if(BITCLEAR(PIN_RESET, P_RESET) && BITCLEAR(PIN_START, P_START))
	{
		both_longpress_cnt = LONGPRESS_HI;
	}
	else
	{
		both_longpress_cnt = 0;
	}

}
