#include <stddef.h>
#include <stdint.h>

#define TOTAL_VALUES 5

static void outl(uint16_t port, uint32_t value) {	
	asm("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port): "memory");
}

void HC_sendProducerArrayAddress(uint32_t address) {
	outl(0xEA, address);
}

void HC_produceValues() {
	int randomValue = 0;
	outl(0xEB, randomValue);
}



void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{

	/* write code here */
	uint32_t producer[TOTAL_VALUES];
	uint32_t value = 0;
	HC_sendProducerArrayAddress((uint32_t)(uintptr_t)producer);
	asm("hlt" : /* empty */ : "a"(42) : "memory");
	*(long *)0x400 = 42;

	for (;;) {
		for(int i = 0; i < TOTAL_VALUES; ++i)
			producer[i] = value++;
		HC_produceValues();
		asm("hlt" : /* empty */ : "a"(42) : "memory");
	}
}
