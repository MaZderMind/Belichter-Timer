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

ISR(PCINT_vect)
{
	if(BITCLEAR(PIN_RESET, P_RESET))
	{
		timer_start();
	}
	else if(BITCLEAR(PIN_START, P_START))
	{
		timer_set_remaining(180);
	}
}
