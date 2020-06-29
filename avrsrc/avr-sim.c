#include "avr-sim.h"
#include <stdlib.h>

void ram_init()
{
	ram_sim_obj = malloc(sizeof(AVR_RAM));
	ram_sim_obj->data = malloc(sizeof(char)*RAM_MEM_SIZE);
}

long avr_alloc(long sz)
{
	return 0;
}

void avr_free(long addr)
{

}

void ram_write(long addr, unsigned char data, long nBytes)
{
	unsigned char* arr = ram_sim_obj->data;

	for(int i=0; i<nBytes; ++i)
	{
		arr[addr+i] = data;
	}
}

void ram_write_float(long addr, float data)
{
	unsigned char* p = (unsigned char*) &data;
	unsigned char* arr = ram_sim_obj->data;
	
	for(int i=0; i<4; ++i)
	{
		arr[addr+i] = p[i];
	}
}

void ram_inc_float(long addr, float data)
{
	float prev = ram_read_float(addr);
	ram_write_float(addr, prev+data);
}

void ram_cpy(long addr, const void* src, long nBytes)
{
	const unsigned char* ptr = (const unsigned char*) src;
	
	for(int i=0; i<nBytes; ++i)
		(ram_sim_obj->data)[addr+i] = ptr[i];
}

unsigned char ram_read(long addr)
{
	return (ram_sim_obj->data)[addr];
}

float ram_read_float(long addr)
{
	float a = 0;
	unsigned char* ptr = (unsigned char*) &a;
	
	for(int i=0; i<4; ++i)
		ptr[i] = (ram_sim_obj->data)[addr+i];
	return a;
}

void ram_close()
{
	free(ram_sim_obj->data);
	free(ram_sim_obj);
}
