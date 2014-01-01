inline void beep_enable(void)
{
	SETBIT(DDRB, PB4);
}

inline void beep_disable(void)
{
	CLEARBIT(DDRB, PB4);
}

inline void beep_cb(void)
{
	TOGGLEBIT(PORTB, PB4);
}
