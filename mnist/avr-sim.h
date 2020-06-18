#ifndef _SIM_AVR
#define _SIM_AVR

#define RAM_MEM_SIZE 721696

typedef struct {
	unsigned char* data;
} AVR_RAM;

AVR_RAM* ram_sim_obj;

void ram_init();

long avr_alloc(long sz);

void avr_free(long addr);

void ram_write(long addr, unsigned char data, long nBytes); // equivalent to memset

void ram_write_float(long addr, float data);

unsigned char ram_read(long addr);

float ram_read_float(long addr); 

void ram_inc_float(long addr, float data); // += operator

void ram_cpy(long addr, const void* src, long nBytes); // memcpy

void ram_close();

#endif
