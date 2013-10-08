void mosfet_init()
{
	// configure as output
	SETBIT(DDR_MOSFET, P_MOSFET);

	// ensure low-state
	CLEARBIT(PORT_MOSFET, P_MOSFET);
}

void mosfet_enable()
{
	SETBIT(PORT_MOSFET, P_MOSFET);
}

void mosfet_disable()
{
	CLEARBIT(PORT_MOSFET, P_MOSFET);
}
