#include <stddef.h>
#include <stdint.h>

#define TOTAL_VALUES 5

static void outl(uint16_t port, uint32_t value) {	
	asm("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port): "memory");
}

void HC_sendConsumerArrayAddress(uint32_t address) {
	outl(0xEC, address);
}

void HC_consumeValues() {
	int randomValue = 0;
	outl(0xED, randomValue);
}

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{
	
	/* Write code here */
	uint32_t consumer[TOTAL_VALUES];
	HC_sendConsumerArrayAddress((uint32_t)(uintptr_t)consumer);
	asm("hlt" : /* empty */ : "a"(42) : "memory");
	*(long *)0x400 = 42;

	for (;;) {
		HC_consumeValues();
		asm("hlt" : /* empty */ : "a"(42) : "memory");
	}
}
