#include <xc.h>

extern unsigned int key_detected;

void __interrupt() isr(void)
{
	if (INT0F == 1)
	{
		key_detected = !key_detected;
		INT0F = 0;
	}
}
