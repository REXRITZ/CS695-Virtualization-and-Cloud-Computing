#include <stddef.h>
#include <stdint.h>

#define BUF_SIZE 20

struct state {
	int32_t prod_p;
	int32_t cons_p;
	uint32_t gen;
	uint32_t available;
	uint32_t buffer[20];
};

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{

	uint32_t buffer[BUF_SIZE];
	int32_t prod_p;
	int32_t cons_p;
	int32_t available;

	/* send buffer address */
	asm volatile("outl %0,%1" : /* empty */ : "a"((uint32_t)(uintptr_t)buffer), "Nd"(0x10): "memory");
	asm("hlt" : /* empty */ : "a"(69) : "memory");

	*(long *)0x400 = 42;

	for (;;) {
		
		/* read prod_p and cons_p */
		asm volatile("inl %1, %0" : "=a"(prod_p) : "Nd"(0x11) : "memory");
		asm volatile("inl %1, %0" : "=a"(cons_p) : "Nd"(0x12) : "memory");
		
		
		/* generate random seed */
		uint32_t seed;
        asm volatile("rdtsc" : "=a"(seed));

		uint32_t maxConsume = seed % 11;

		if(maxConsume == 0) {
			/* send updated prod_p to hypervisor */
			asm volatile("outl %0,%1" : /* empty */ : "a"(cons_p), "Nd"(0x13): "memory");
			asm("hlt" : /* empty */ : "a"(42) : "memory");
			continue;
		}
		
		if(prod_p == -1)
			available = 0;
		else
			available = (prod_p - cons_p + BUF_SIZE) % BUF_SIZE;
		
		/* consume here */
		while(maxConsume > 0 && available > 0) {
			cons_p = (cons_p + 1) % BUF_SIZE;
			available--;
			maxConsume--;
		}

		// /* send updated cons_p to hypervisor */
		asm volatile("outl %0,%1" : /* empty */ : "a"(cons_p), "Nd"(0x13): "memory");

		asm("hlt" : /* empty */ : "a"(42) : "memory");
	}
}
