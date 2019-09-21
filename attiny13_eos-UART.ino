/**
 * ATTiny13[and more] - Cannon EOS Shutter Timer
 * 
 * (c) 2019 Petr KLOSKO
 * 
 * UART code and CLI based on 
 *      ATtiny13/021 -  Simple text CLI (Command Line Interface) via UART.
 *      by Łukasz Podkalicki <lpodkalicki@gmail.com>
 *      [https://blog.podkalicki.com/100-projects-on-attiny13/]
 *
 *
 *            ATTiny13 1.2MHz internal
 *                              _____
 *                     RESET  -|     |- VCC
 *   TX(see uart.h) <--  PB3  -|     |- PB2 --> 470Ohm -> FOCUS Optocoupler LED
 *   RX(see uart.h) <--  BP4  -|     |-    
 *                         GN -|_____|- PB0 --> 470Ohm -> SHUTTER Optocoupler LED 
 *       
 *       VCC[CR2032]      RESET        VCC         
 *        |               __|__         |
 *       ===  100nF       button      100kOhm 
 *        |               -----         |
 *       GND                |         RESET 
 *                         GND  
 */
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define	BUFFER_SIZE	(8)
#define	ARGV_SIZE	(2)
#define	PROG_NAME_SIZE	(2)

#define FOCUS_DELAY    500
#define PHOTO_DELAY    100

#define  PHOTO_PIN PB0
#define  START_PIN PB1
#define  FOCUS_PIN PB2

typedef void (*program_hook_t)(void);

typedef struct s_program {
	char name[PROG_NAME_SIZE];
	program_hook_t hook;
} program_t;

static char buffer[BUFFER_SIZE];
static const char *argv[ARGV_SIZE] = {0, 0};
static uint8_t argc = 0;

static uint8_t readline(void);
static void parse(void);
static void execute(void);
static int8_t xscmp(const char *s1, const char *s2);
static uint16_t xscanu(const char *value);

uint8_t Interval = 5;
uint8_t Focus    = 1;
bool CLI         = true;  // Wait for UART command after RESET
 
/**
 * Command "?".
 * No arguments required. Print help.
 */
/*static void
prog_help_hook(void){
	uart_puts("..help\n");
}
*/

/*
 * Command "i"
 * Set Interval to [ARG] second
 */
static void
prog_interval_hook(void){
  if (xscanu(argv[1])){
    Interval = xscanu(argv[1]);
  }
  uart_puts("I");
  uart_putu(Interval);  
  uart_puts("\n");
}

/*
 * Command "f".
 * Set Focus to [ARG] [0 | 1]
 */
static void
prog_focus_hook(void){
  if (xscanu(argv[1])){
    Focus = xscanu(argv[1]);
  }
  uart_puts("F");
  uart_putu(Focus);  
  uart_puts("\n");
}

/*
 * Command "r".
 * Run Loop = wait for Interval sec, then Focus and Shutter
 */
static void
prog_run_hook(void){
  CLI = false;
  uart_puts("GO\n");
//  uart_putu(Run);  
//  uart_puts("\n");
}

const program_t programs[3] PROGMEM = {
  {"i", prog_interval_hook},
  {"f", prog_focus_hook},
  {"s", prog_run_hook},
	/* Add here your hook! */
};

int
main(void){
  /* setup */
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  ACSR = (1<<ACD); //Disable the analog comparator

//  CLI = true;
  DDRB = 0b00000101;              // set LED pin as OUTPUT , Start pin as INPUT
  PORTB = 0b00000010;             // set all pins to LOW , Start pin to HI
  CLI = ( (PINB & 0b00000010) != 0 );  // read AUTO START PIN
  
  /* Print some info after reset*/
  prog_interval_hook();
  prog_focus_hook();
  uart_puts("OK\n");
  
	/* loop */
	while (1) {
    if (CLI){
      /* Wait for commands - DEFAULT after RESET*/
		  if (readline() > 0) {
			  parse();
			  execute();
		  }
    }else{
      if (Focus != 0) {
        PORTB ^= _BV(FOCUS_PIN);  // toggle FOCUS LED pin
        _delay_ms(FOCUS_DELAY);
      }  
      PORTB ^= _BV(PHOTO_PIN);  // toggle SHUTTER LED pin
      _delay_ms(PHOTO_DELAY);
      PORTB = 0b00000010;             // set all pins to LOW
      delay(Interval*1000);
	  }
	} 
}

/**
 * Read line from UART and store in buffer.
 */
uint8_t
readline(void){
	char c;
	uint8_t sreg, len = 0;

	while (len < BUFFER_SIZE) {
		c = uart_getc();     
		if (c == '\n' || c == '\r') {
			buffer[len] = 0;
			break;
		}
		buffer[len++] = c;
//    uart_putc(c);
	}

	return len;
}

/**
 * Parse arguments.
 * Parse buffer, extract argument list to argv[]
 * and set the number of arguments in argc.
 */
void
parse(void){
	char *p;
	argc = 0;
	argv[argc++] = buffer;
	p = buffer;
	while (*p != '\0' && argc < ARGV_SIZE) {
		if (*p == ' ') {
			*p = 0;
			argv[argc++] = p + 1;
		}
		p++;
	}
}

/**
 * Execute program.
 * Compare with available program names and execute callback,
 * otherwise print error.
 */
void
execute(void){	uint8_t i;
	char name[6];
	program_hook_t hook;

	for (i = 0; i < sizeof(programs); ++i) {
		memcpy_P(name, &programs[i].name, PROG_NAME_SIZE);
		if (xscmp(argv[0], name) == 0) {
			hook = (program_hook_t)pgm_read_word(&programs[i].hook);
			hook();
			return;
		}
	}

	uart_puts("ERR\n");
//  disabled - no space left on attiny13
//	uart_puts(argv[0]);
//	uart_putc('\n');
}

/**
 * Compare two strings.
 */
int8_t
xscmp(const char *s1, const char *s2){
        while (*s1 != '\0' && *s1 == *s2 ) {s1++; s2++;}
        return (*s1 - *s2);
}

/**
 * Convert string to integer.
 */
uint16_t
xscanu(const char *value){
	const char *p;
  uint16_t x = 0;
	p = value;
	while (*p && (*p < 48 || *p > 57)) {p++;}
	while (*p && (*p > 47 && *p < 58)) {x = (x << 1) + (x << 3) + *(p++) - 48;}
  return x;
}
