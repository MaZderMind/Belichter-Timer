void mosfet_init(void)
{
	// configure as output
	SETBIT(DDR_MOSFET, P_MOSFET);

	// ensure low-state
	CLEARBIT(PORT_MOSFET, P_MOSFET);
}

void mosfet_enable(void)
{
	SETBIT(PORT_MOSFET, P_MOSFET);
}

void mosfet_disable(void)
{
	CLEARBIT(PORT_MOSFET, P_MOSFET);
}
