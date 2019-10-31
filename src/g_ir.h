/**
 * g_ir.h
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

#ifndef _G_IR_H_
#define _G_IR_H_

#include "g_common.h"

#define G_IR_MEMORY_ACTIVATE 0
#define G_IR_MEMORY_TRAIN    1
#define G_IR_MEMORY_END      2

#define G_IR_PRECISION_NONE   0
#define G_IR_PRECISION_FLOAT  1
#define G_IR_PRECISION_DOUBLE 2
#define G_IR_PRECISION_FIXED  3

#define G_IR_COSTFNC_NONE          0
#define G_IR_COSTFNC_QUADRATIC     1
#define G_IR_COSTFNC_EXPONENTIAL   2
#define G_IR_COSTFNC_CROSS_ENTROPY 3

#define G_IR_ACTIVATION_NONE    0
#define G_IR_ACTIVATION_RELU    1
#define G_IR_ACTIVATION_LINEAR  2
#define G_IR_ACTIVATION_SOFTMAX 3
#define G_IR_ACTIVATION_SIGMOID 4

struct g_ir {
	int batch;
	int layers;
	double eta;
	int costfnc;
	const char *module;
	const char *prefix;
	struct {
		int whole;
		int fraction;
		int precision;
	} memory[G_IR_MEMORY_END];
	struct {
		int size;
		int activation;
	} *nodes;
};

const struct g_ir *g_ir_parse(const char *pathname);

void g_ir_destroy(void);

/*-----------------------------------------------------------------------------
 * Lexer/Parser Backend
 *---------------------------------------------------------------------------*/

extern int yylineno;
extern int yylex(void);
extern int yyparse(void);
extern int yylex_destroy(void);
extern void yyrestart(FILE *file);
extern void yyerror(const char *format, ...);

int g_ir_top(void);
int g_ir_precision(long whole1,
		   long fraction1,
		   long precision1,
		   long whole2,
		   long fraction2,
		   long precision2);
int g_ir_module(const char *s);
int g_ir_prefix(const char *s);
int g_ir_costfnc(long costfnc);
int g_ir_batch(long batch);
int g_ir_eta(double eta);
int g_ir_input(long size);
int g_ir_output(long size, long activation);
int g_ir_hidden(long size, long activation);
void *g_ir_malloc(size_t n);
char *g_ir_strdup(const char *s_);

#endif /* _G_IR_H_ */
