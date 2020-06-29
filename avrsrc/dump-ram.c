#include "avr-sim.h"
#include "dump-ram.h"
#include "test.h"
#include <stdio.h>

void dump_ram()
{
	FILE* file = fopen(RAM_MEM_PATH, "w+");
	
	for(int i=0; i<RAM_MEM_SIZE; ++i)
	{
		fputc(ram_read(i), file);
	}
	fclose(file);
}

void read_ram()
{
	FILE* file = fopen(RAM_MEM_PATH, "r");
	
	size_t limit = test_memory_size();
	for(long i=0; i<limit; ++i)
	{
		ram_write(i, fgetc(file), 1);
	}
	fclose(file);
}