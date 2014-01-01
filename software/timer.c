#define CP_VALUE 15625
#define CP_VALUE_FAST CP_VALUE/6
#define CP_VALUE_SLOW CP_VALUE*2

#define TIME_MAX 5999

uint16_t ee_runtime EEMEM = 270;
uint16_t runtime;

void timer_set_remaining(uint16_t time);

inline void timer_init(void)
{
	// enable timer1 for clock cycle

	// we want at exactly 1Hz interrupt cycle
	// so we set a prescaling divider of 256 and run the counter with a freqency of 15.625 kHz
	// to get down to 1 Hz from there we need a divider of 15625 which we set via the Output-Compare Value

	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	runtime = eeprom_read_word(&ee_runtime);
	timer_set_remaining(runtime);

	// prescaler to 256
	SETBIT(TCCR1B, CS12);

	// auto-clear the counter on output-compare match (timer 0)
	SETBIT(TCCR1B, WGM12);

	// set output-compare-value to 15625
	OCR1A = CP_VALUE;

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
#define TIMER_FINISHED 7
#define TIMER_OPEN 8

// the maximum time displayable is 99 Minutes and 59 Seconds, which equals 5999 Seconds. Therefore we need an 16 bit counter variable
volatile uint16_t remaining_time = 0;
volatile uint8_t timer_status = TIMER_WAITING_A;


void timer_set_remaining(uint16_t time)
{
	// guard
	if(time > TIME_MAX)
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

inline void timer_enter_setmode(void)
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

inline void timer_leave_setmode(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	eeprom_write_word(&ee_runtime, runtime);

	timer_status = TIMER_LEAVE_SETMODE;
	display_set_done();

	// reset timer-value to 0
	OCR1A = CP_VALUE_SLOW;
	TCNT1 = 0;

	// restore system state
	SREG = sreg_tmp;
}

inline uint16_t timer_modify(int8_t delta)
{
	if(delta > 0 && runtime == TIME_MAX) return runtime;
	if(delta < 0 && runtime == 0) return runtime;

	return (runtime += delta);
}

void timer_reset_requested(void)
{
	// set runtime if system is in waiting state
	if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B || timer_status == TIMER_FINISHED)
		timer_set_remaining(runtime);
	else if(timer_status == TIMER_SETMODE_A || timer_status == TIMER_SETMODE_B)
		display_set_time(timer_modify(-1));
}

void timer_startstop_requested(void)
{
	// set runtime if system is in waiting state
	if(timer_status == TIMER_RUNNING)
		timer_stop();
	else if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B)
		timer_start();
	else if(timer_status == TIMER_SETMODE_A || timer_status == TIMER_SETMODE_B)
		display_set_time(timer_modify(+1));
	else if(timer_status == TIMER_OPEN)
		display_set_open();
}

inline void timer_setmode_requested(void)
{
	if(timer_status == TIMER_SETMODE_A || timer_status == TIMER_SETMODE_B || timer_status == TIMER_ENTER_SETMODE)
		timer_leave_setmode();
	if(timer_status == TIMER_WAITING_A || timer_status == TIMER_WAITING_B || timer_status == TIMER_FINISHED)
		timer_enter_setmode();
}

inline void timer_case_set_opened(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	timer_status = TIMER_OPEN;
	display_set_open();
	mosfet_disable();

	// restore system state
	SREG = sreg_tmp;
}

inline void timer_case_set_closed(void)
{
	// store system state and disable interrupts
	uint8_t sreg_tmp = SREG;
	cli();

	if(timer_status == TIMER_OPEN)
	{
		display_set_time(remaining_time);
		timer_status = TIMER_WAITING_A;
	}

	// restore system state
	SREG = sreg_tmp;
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
		display_set_time(runtime);
	}
	else if(timer_status == TIMER_LEAVE_SETMODE)
	{
		OCR1A = CP_VALUE;
		timer_status = TIMER_WAITING_A;
		timer_set_remaining(runtime);
	}
	else if(timer_status == TIMER_SETMODE_A)
	{
		display_set_empty();
		timer_status = TIMER_SETMODE_B;
	}
	else if(timer_status == TIMER_SETMODE_B)
	{
		display_set_time(runtime);
		timer_status = TIMER_SETMODE_A;
	}
	else if(timer_status == TIMER_RUNNING)
	{
		if(remaining_time == 1)
		{
			timer_status = TIMER_FINISHED;
			// disable mosfet
			mosfet_disable();

			display_set_done();
		}
		else
		{
			display_set_time(--remaining_time);
		}
	}
}
