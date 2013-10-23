#define CP_VALUE 3906
#define CP_VALUE_FAST CP_VALUE/6
#define CP_VALUE_SLOW CP_VALUE*2

void timer_init(void)
{
	// enable timer1 for clock cycle

	// we want at exactly 1Hz interrupt cycle
	// so we set a prescaling divider of 1024 and run the counter with a freqency of 3906.25 Hz
	// to get down to 1 Hz fro mthere we would need a divider of 3906.25... which is obviously not simple to set
	// so what we'll to is set the counter to 3906 ignore the small off-error. that's okay for a 3 1/2 minute countdown

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// prescaler to 1024
	SETBITS(TCCR1B, BIT(CS12) | BIT(CS10));

	// auto-clear the counter on output-compare match (timer 0)
	SETBIT(TCCR1B, WGM12);

	// set output-compare-value to 3906
	OCR1A = 3906;

	// enable output-compare interrupt (timer 0, compare A)
	SETBIT(TIMSK, OCIE1A);

	// restore system state
	SREG = sreg_tmp;
}

#define TIMER_RUNNING 0
#define TIMER_WAITING_A 1
#define TIMER_WAITING_B 2
#define TIMER_ENTER_SETMODE 3
#define TIMER_LEAVE_SETMODE 4
#define TIMER_SETMODE_A 5
#define TIMER_SETMODE_B 6

// the maximum time displayable is 99 Minutes and 59 Seconds, which equals 5999 Seconds. Therefore we need an 16 bit counter variable
volatile uint16_t remaining_time = 0;
volatile uint8_t timer_status = TIMER_WAITING_A;


void timer_set_remaining(uint16_t time)
{
	// guard
	if(time > 5999)
		return;

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	// enable mosfet
	mosfet_disable();

	remaining_time = time;

	if(time == 0)
	{
		// set display to "done"
		display_set_done();
	}
	else
	{
		// update displayed value
		display_set_time(time);

		// set to waiting
		timer_status = TIMER_WAITING_A;

	}


	// reset timer-value to 0
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}

void timer_start(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	mosfet_enable();

	timer_status = TIMER_RUNNING;
	display_set_time(remaining_time);

	// reset timer-value to 0
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}

void timer_stop(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	mosfet_disable();

	timer_status = TIMER_WAITING_A;

	// reset timer-value to 0
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}

void timer_enter_setmode(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	mosfet_disable();

	timer_status = TIMER_ENTER_SETMODE;
	display_set_set();

	// reset timer-value to 0
	OCR1A = CP_VALUE_SLOW;
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}

void timer_leave_setmode(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	mosfet_disable();

	timer_status = TIMER_LEAVE_SETMODE;
	display_set_done();

	// reset timer-value to 0
	OCR1A = CP_VALUE_SLOW;
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}


void timer_reset_requested(void)
{
	// set runtime if system is in waiting state
	if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B)
		timer_set_remaining(RUNTIME);
}

void timer_startstop_requested(void)
{
	// set runtime if system is in waiting state
	if(timer_status == TIMER_RUNNING)
		timer_stop();
	else if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B)
		timer_start();
}

void timer_setmode_requested(void)
{
	if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B)
		timer_enter_setmode();
	else if(timer_status == TIMER_SETMODE_A || timer_status == TIMER_SETMODE_B || timer_status == TIMER_ENTER_SETMODE)
		timer_leave_setmode();
}


ISR(TIMER1_COMPA_vect)
{
	if(timer_status == TIMER_WAITING_A)
	{
		display_set_empty();
		timer_status = TIMER_WAITING_B;
	}
	else if(timer_status == TIMER_WAITING_B)
	{
		display_set_time(remaining_time);
		timer_status = TIMER_WAITING_A;
	}
	else if(timer_status == TIMER_ENTER_SETMODE)
	{
		OCR1A = CP_VALUE_FAST;
		timer_status = TIMER_SETMODE_A;
		display_set_time(RUNTIME);
	}
	else if(timer_status == TIMER_LEAVE_SETMODE)
	{
		OCR1A = CP_VALUE;
		timer_status = TIMER_WAITING_A;
		display_set_time(RUNTIME);
	}
	else if(timer_status == TIMER_SETMODE_A)
	{
		display_set_empty();
		timer_status = TIMER_SETMODE_B;
	}
	else if(timer_status == TIMER_SETMODE_B)
	{
		display_set_time(RUNTIME);
		timer_status = TIMER_SETMODE_A;
	}
	else if(remaining_time == 1)
	{
		// disable mosfet
		mosfet_disable();

		display_set_done();
	}
	else
	{
		display_set_time(--remaining_time);
	}
}
