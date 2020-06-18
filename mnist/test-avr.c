/*
 * Auto Generated by The Gravity Compiler - Fri Feb 21 03:20:44 2020
 * Copyright (C) Tony Givargis, 2019-2020
 * 
 * Modified 2020 to be compatible with AVR RAM
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "test-avr.h"
#include "avr-sim.h"

static void _initialize_() {
  { /* RANDOM */
    float r;
    uint32_t i;
    for (i=0; i<78400; ++i) {
      r = (float)rand() / RAND_MAX;
      ram_write_float(0+4*i, -0.006787 + r * 0.013575);
    }
  }

  { /* CLEAR */
    ram_write(313600, 0, 100 * sizeof (float));
  }

  { /* RANDOM */
    float r;
    uint32_t i;
    for (i=0; i<10000; ++i) {
      r = (float)rand() / RAND_MAX;
      ram_write_float(314000+4*i, -0.030000 + r * 0.060000);
    }
  }

  { /* CLEAR */
    ram_write(354000, 0, 100 * sizeof (float));
  }

  { /* RANDOM */
    float r;
    uint32_t i;
    for (i=0; i<1000; ++i) {
      r = (float)rand() / RAND_MAX;
      ram_write_float(354400+4*i, -0.054545 + r * 0.109091);
    }
  }

  { /* CLEAR */
    ram_write(358400, 0, 10 * sizeof (float));
  }

  { /* RET */
    return;
  }
}

static float *_activate_(const float *x_) {
  { /* COPYX */
    ram_cpy(716880, x_, 784 * sizeof (float));
  }

  { /* MAC1 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
	  ram_write_float(720016+4*i, 0.0);
      for (j=0; j<784; ++j) {
	    const float A = ram_read_float(0 + 4*(i * 784 + j));
		const float B = ram_read_float(716880 + 4*j);
        ram_inc_float(720016 + 4*i, A * B);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(313600+4*i);
	  ram_inc_float(720016 + 4*i, B);
    }
  }

  { /* RELU */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  float za = ram_read_float(720016+4*i);
      if (0.0 >= za) {
        ram_write_float(720016+4*i, 0.0);
      }
    }
  }

  { /* MAC1 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
	  ram_write_float(720816+4*i, 0.0);
      for (j=0; j<100; ++j) {
		const float A = ram_read_float(314000 + 4*(i * 100 + j));
		const float B = ram_read_float(720016 + 4*j);
		ram_inc_float(720816+4*i, A * B);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(354000 + 4*i);
	  ram_inc_float(720816 + 4*i, B);
    }
  }

  { /* RELU */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  float za = ram_read_float(720816+4*i);
      if (0.0 >= za) {
        ram_write_float(720816+4*i, 0.0);
      }
    }
  }

  { /* MAC1 */
    uint32_t i, j;
    for (i=0; i<10; ++i) {
	  ram_write_float(721616+4*i, 0.0);
      for (j=0; j<100; ++j) {
		const float A = ram_read_float(354400 + 4*(i * 100 + j));
		const float B = ram_read_float(720816 + 4*j);
		ram_inc_float(721616+4*i, A * B);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<10; ++i) {
	  const float B = ram_read_float(358400 + 4*i);
	  ram_inc_float(721616 + 4*i, B);
    }
  }

  { /* SOFTMAX */
    float max=ram_read_float(721616), sum=0.0;
    uint32_t i;
    for (i=1; i<10; ++i) {
	  float za = ram_read_float(721616 + 4*i);
      if (max < za) {
        max = za;
      }
    }
	float* za_arr = malloc(sizeof(float) * 10);
    for (i=0; i<10; ++i) {
	  ram_inc_float(721616 + 4*i, -max);
	  za_arr[i] = ram_read_float(721616 + 4*i);
      sum += (float)exp(za_arr[i]);
	  
    }
    for (i=0; i<10; ++i) {
      ram_write_float(721616 + 4*i, (float)exp(za_arr[i]) / sum);
    }
  }

  { /* RETARG */
	float* ret_arr = malloc(sizeof(float) * 10);
	
	for(int i=0; i<10; ++i)
		ret_arr[i] = ram_read_float(721616 + 4*i);
    return ret_arr;
  }
}

static void _backprop_(const float *y_) {
  { /* SUBY */
    uint32_t i;
    for (i=0; i<10; ++i) {
	  const float A = ram_read_float(721616 + 4*i);
	  ram_write_float(721656 + 4*i, A - y_[i]);
    }
  }

  { /* MAC2 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
	  ram_write_float(721216 + 4*i, 0.0);
      for (j=0; j<10; ++j) {
		const float A = ram_read_float(354400 + 4*(j * 100 + i));
		const float B = ram_read_float(721656 + 4*j);
		ram_inc_float(721216 + 4*i, A * B);
      }
    }
  }

  { /* RELUD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(720816 + 4*i);
      if (0.0 >= B) {
		ram_write_float(721216, 0.0);
      }
    }
  }

  { /* MAC2 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
	  ram_write_float(720416 + 4*i, 0.0);
      for (j=0; j<100; ++j) {
		const float A = ram_read_float(314000 + 4*(j * 100 + i));
		const float B = ram_read_float(721216 + 4*j);
		ram_inc_float(720416 + 4*i, A * B);
      }
    }
  }

  { /* RELUD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(720016 + 4*i);
      if (0.0 >= B) {
		ram_write_float(720416 + 4*i, 0.0);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(720416 + 4*i);
	  ram_inc_float(672040 + 4*i, B);
    }
  }

  { /* MAC3 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
      for (j=0; j<784; ++j) {
		const float B = ram_read_float(720416 + 4*i);
		const float C = ram_read_float(716880 + 4*j);
		ram_inc_float(358440 + 4*(i * 784 + j), B * C);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(721216 + 4*i);
	  ram_inc_float(712440 + 4*i, B);
    }
  }

  { /* MAC3 */
    uint32_t i, j;
    for (i=0; i<100; ++i) {
      for (j=0; j<100; ++j) {
		const float B = ram_read_float(721216 + 4*i);
		const float C = ram_read_float(720016 + 4*j);
		ram_inc_float(672440 + 4*(i * 100 + j), B * C);
      }
    }
  }

  { /* ADD */
    uint32_t i;
    for (i=0; i<10; ++i) {
	  const float B = ram_read_float(721656 + 4*i);
	  ram_inc_float(716840 + 4*i, B);
    }
  }

  { /* MAC3 */
    uint32_t i, j;
    for (i=0; i<10; ++i) {
      for (j=0; j<100; ++j) {
		const float B = ram_read_float(721656 + 4*i);
		const float C = ram_read_float(720816 + 4*j);
		ram_inc_float(712840 + 4*(i * 100 + j), B * C);
      }
    }
  }

  { /* RET */
    return;
  }
}

static void _train_(const float *x_, const float *y_) {
  { /* CLEAR */
    ram_write(358440, 0, 89610 * sizeof (float));
  }

  { /* BATCHLOOP */
    uint32_t i;
    for (i=0; i<8; ++i) {
        float* ret_arr = _activate_(x_ + i * 784);
		free(ret_arr);
        _backprop_(y_ + i * 10);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<78400; ++i) {
	  const float B = ram_read_float(358440 + 4*i);
	  ram_inc_float(0 + 4*i, B * -0.012500);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(672040 + 4*i);
	  ram_inc_float(313600 + 4*i, B * -0.012500);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<10000; ++i) {
	  const float B = ram_read_float(672440 + 4*i);
	  ram_inc_float(314000 + 4*i, B * -0.012500);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<100; ++i) {
	  const float B = ram_read_float(712440 + 4*i);
	  ram_inc_float(354000 + 4*i, B * -0.012500);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<1000; ++i) {
	  const float B = ram_read_float(712840 + 4*i);
	  ram_inc_float(354400 + 4*i, B * -0.012500);
    }
  }

  { /* MAC4 */
    uint32_t i;
    for (i=0; i<10; ++i) {
	  const float B = ram_read_float(716840 + 4*i);
	  ram_inc_float(358400 + 4*i, B * -0.012500);
    }
  }

  { /* RET */
    return;
  }
}

int test_version(void) {
  return 10;
}

size_t test_memory_size(void) {
  return 721696;
}

size_t test_memory_hard(void) {
  return 358440;
}

void test_initialize() {
  _initialize_();
}

void *test_activate(const void *x) {
  return _activate_((const float *)x);
}

void test_train(const void *x, const void *y) {
  _train_((const float *)x, (const float *)y);
}

