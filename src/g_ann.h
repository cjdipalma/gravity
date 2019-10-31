/**
 * g_ann.h
 * Copyright (C) Tony Givargis, 2019
 *
 * This file is part of The Gravity Compiler.
 *
 * The Gravity Compiler is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. The Gravity Compiler is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _G_ANN_H_
#define _G_ANN_H_

#include "g_ir.h"

#define G_ANN_PRECISION_NONE   G_IR_PRECISION_NONE
#define G_ANN_PRECISION_FLOAT  G_IR_PRECISION_FLOAT
#define G_ANN_PRECISION_DOUBLE G_IR_PRECISION_DOUBLE
#define G_ANN_PRECISION_FIXED  G_IR_PRECISION_FIXED

#define G_ANN_MEMORY_ACTIVATE G_IR_MEMORY_ACTIVATE
#define G_ANN_MEMORY_TRAIN    G_IR_MEMORY_TRAIN
#define G_ANN_MEMORY_END      G_IR_MEMORY_END

#define G_ANN_PROGRAM_ACTIVATEX 0
#define G_ANN_PROGRAM_ACTIVATE  1
#define G_ANN_PROGRAM_BACKPROP  2
#define G_ANN_PROGRAM_TRAIN     3
#define G_ANN_PROGRAM_END       4

#define G_ANN_PROGRAM_INST_RET         1
#define G_ANN_PROGRAM_INST_RETARG      2
#define G_ANN_PROGRAM_INST_BATCHLOOP   3
#define G_ANN_PROGRAM_INST_CLEAR      11
#define G_ANN_PROGRAM_INST_COPYX      12
#define G_ANN_PROGRAM_INST_MUL1       13
#define G_ANN_PROGRAM_INST_MUL2       14
#define G_ANN_PROGRAM_INST_MUL3       15
#define G_ANN_PROGRAM_INST_MULS       16
#define G_ANN_PROGRAM_INST_ADD        17
#define G_ANN_PROGRAM_INST_SUBY       18
#define G_ANN_PROGRAM_INST_RELU      101
#define G_ANN_PROGRAM_INST_LINEAR    102
#define G_ANN_PROGRAM_INST_SOFTMAX   103
#define G_ANN_PROGRAM_INST_SIGMOID   104
#define G_ANN_PROGRAM_INST_RELUD    1001
#define G_ANN_PROGRAM_INST_LINEARD  1002
#define G_ANN_PROGRAM_INST_SOFTMAXD 1003
#define G_ANN_PROGRAM_INST_SIGMOIDD 1004

struct g_ann {
	int layers;
	const char *module;
	const char *prefix;
	struct g_ann_memory {
		int whole;
		int fraction;
		int precision;
		uint64_t unit;
		uint64_t size;
		uint64_t *w;
		uint64_t *b;
		uint64_t *a_;
		uint64_t *d_;
		uint64_t *w_;
		uint64_t *b_;
	} memory[G_ANN_MEMORY_END];
	struct g_ann_program {
		int size;
		struct g_ann_program_inst {
			int opc;
			int whole;
			int fraction;
			int precision;
			union {
				uint64_t i;
				double r;
			} arg[5];
		} *inst;
	} program[G_ANN_PROGRAM_END];
};

struct g_ann *g_ann_open(const struct g_ir *ir);

void g_ann_close(struct g_ann *an);

#endif /* _G_ANN_H_ */
