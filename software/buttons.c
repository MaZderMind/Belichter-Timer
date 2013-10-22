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

#define DEBOUNCE_HI 3
#define DEBOUNCE_LO 0

uint8_t reset_cnt = 0, start_cnt = 0;
void button_debounce_isr(void)
{
	if(reset_cnt > 0)
	{
		if(--reset_cnt == DEBOUNCE_LO)
			timer_set_remaining(RUNTIME);
	}

	if(start_cnt > 0)
	{
		if(--start_cnt == DEBOUNCE_LO)
			timer_start();
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
		reset_cnt = DEBOUNCE_LO;
	}

	if(BITCLEAR(PIN_START, P_START))
	{
		start_cnt = DEBOUNCE_HI;
	}
	else
	{
		start_cnt = DEBOUNCE_LO;
	}
}
