#include "avr-sim.h"
#include <stdio.h>

int main()
{
	long address = 21;

	ram_init();
	float a = 0.6f;
	ram_cpy(address, &a, 4);
	ram_write_float(address+4, 294.345f);
	ram_inc_float(address+4, -88.765f);

	for(int i=0; i<5; ++i)
	{
		//printf("%d\n", ram_read(address+i));
	}
	printf("\n");

	for(int i=0; i<2; ++i)
		printf("%f\n", ram_read_float(address+i));
	printf("%f\n", ram_read_float(address+4));

	ram_close();
	return 0;
}
