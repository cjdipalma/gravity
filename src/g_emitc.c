/**
 * g_emitc.c
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

#include "g_emitc.h"

#define UL(x) ((unsigned long)(x))

#define P print

static const char *capitalize(const char *s_) {
	size_t i;
	char *s;

	s = g_malloc(g_strlen(s_) + 1);
	if (!s) {
		G_DEBUG(0);
		return 0;
	}
	for (i=0; i<g_strlen(s_); ++i) {
		s[i] = toupper(s_[i]);
	}
	s[i] = 0;
	return s;
}

static int print(FILE *file, const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	if (0 > vfprintf(file, format, ap)) {
		va_end(ap);
		G_DEBUG(G_ERR_FILE);
		return -1;
	}
	va_end(ap);
	return 0;
}

static const char *precision(const struct g_ann_program_inst *inst) {
	switch (inst->precision) {
	case G_ANN_PRECISION_FLOAT : return "float";
	case G_ANN_PRECISION_DOUBLE: return "double";
	case G_ANN_PRECISION_FIXED : break; /* FIX : not implemented */
	default /*--------------*/ : break;
	}
	G_DEBUG(G_ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return 0;
}

static const char *type(uint64_t x) {
	if (0xffffffff >= x) {
		return "unsigned";
	}
	return "unsigned long";
}

static int inst_ret(const struct g_ann_program_inst *inst, FILE *file) {
	G_UNUSED(inst);
	if (P(file,
	      "  { /* RET */\n"
	      "    return;\n"
	      "  }\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_retarg(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* RETARG */\n"
	      "    return (%s *)( m_ + %lu );\n"
	      "  }\n",
	      precision(inst),
	      UL(inst->arg[0].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_batchloop(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* BATCHLOOP */\n"
	      "    %s i;\n"
	      "    for (i=0; i<%lu; ++i) {\n"
	      "        _activate_(m_, x_ + i * %lu);\n"
	      "        _backprop_(m_, y_ + i * %lu);\n"
	      "    }\n"
	      "  }\n\n",
	      type(inst->arg[0].i * G_MAX(inst->arg[1].i, inst->arg[2].i)),
	      UL(inst->arg[0].i),
	      UL(inst->arg[1].i),
	      UL(inst->arg[2].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_clear(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* CLEAR */\n"
	      "    memset(m_ + %lu, 0, %lu);\n"
	      "  }\n\n",
	      UL(inst->arg[0].i),
	      UL(inst->arg[1].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_copyx(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* COPYX */\n"
	      "    memcpy(m_ + %lu, x_, %lu);\n"
	      "  }\n\n",
	      UL(inst->arg[0].i),
	      UL(inst->arg[1].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_mul1(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* MUL1 */\n"
	      "    %s *z = (%s *)( m_ + %lu );\n"
	      "    const %s *A = (const %s *)( m_ + %lu );\n"
	      "    const %s *B = (const %s *)( m_ + %lu );\n"
	      "    %s i, j;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[2].i),
	      type(G_MAX(inst->arg[3].i, inst->arg[4].i))) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      z[i] = 0.0;\n"
	      "      for (j=0; j<%lu; ++j) {\n"
	      "        z[i] += A[i * %lu + j] * B[j];\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[3].i),
	      UL(inst->arg[4].i),
	      UL(inst->arg[4].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_mul2(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* MUL2 */\n"
	      "    %s *z = (%s *)( m_ + %lu );\n"
	      "    const %s *A = (const %s *)( m_ + %lu );\n"
	      "    const %s *B = (const %s *)( m_ + %lu );\n"
	      "    %s i, j;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[2].i),
	      type(G_MAX(inst->arg[3].i, inst->arg[4].i))) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      z[i] = 0.0;\n"
	      "      for (j=0; j<%lu; ++j) {\n"
	      "        z[i] += A[j * %lu + i] * B[j];\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[4].i),
	      UL(inst->arg[3].i),
	      UL(inst->arg[4].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_mul3(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* MUL3 */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    const %s *B = (const %s *)( m_ + %lu );\n"
	      "    const %s *C = (const %s *)( m_ + %lu );\n"
	      "    %s i, j;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[2].i),
	      type(G_MAX(inst->arg[3].i, inst->arg[4].i))) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      for (j=0; j<%lu; ++j) {\n"
	      "        za[i * %lu + j] += B[i] * C[j];\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[3].i),
	      UL(inst->arg[4].i),
	      UL(inst->arg[4].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_muls(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* MULS */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      type(inst->arg[1].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      za[i] += %f;\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[1].i),
	      inst->arg[2].r)) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_add(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* ADD */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    const %s *B = (const %s *)( m_ + %lu );\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      type(inst->arg[2].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      za[i] += B[i];\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[2].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_suby(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* SUBY */\n"
	      "    %s *z = (%s *)( m_ + %lu );\n"
	      "    const %s *A = (const %s *)( m_ + %lu );\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      type(inst->arg[2].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      z[i] = A[i] - y_[i];\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[2].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_relu(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* RELU */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      type(inst->arg[1].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      if (0.0 >= za[i]) {\n"
	      "	       za[i] = 0.0;\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[1].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_linear(const struct g_ann_program_inst *inst, FILE *file) {
	G_UNUSED(inst);
	if (P(file,
	      "  { /* LINEAR */\n"
	      "    /* nothing to do */\n"
	      "  }\n\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_softmax(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* SOFTMAX */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    %s max=0.0, sum=0.0;\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      type(inst->arg[1].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      if (max < za[i]) {\n"
	      "        max = za[i];\n"
	      "      }\n"
	      "    }\n"
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      za[i] -= max;\n"
	      "      sum += (%s)exp(za[i]);\n"
	      "    }\n"
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      za[i] = (%s)exp(za[i]) / sum;\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[1].i),
	      UL(inst->arg[1].i),
	      precision(inst),
	      UL(inst->arg[1].i),
	      precision(inst))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_sigmoid(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* SIGMOID */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    %s zee;\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      type(inst->arg[1].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      if (0.0 <= za[i]) {\n"
	      "        zee = (%s)exp(-za[i]);\n"
	      "        za[i] = 1.0 / (1.0 + zee);\n"
	      "      }\n"
	      "      else {\n"
	      "        zee = (%s)exp(za[i]);\n"
	      "        za[i] = zee / (1.0 + zee);\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[1].i),
	      precision(inst),
	      precision(inst))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_relud(const struct g_ann_program_inst *inst, FILE *file) {
	if (P(file,
	      "  { /* RELUD */\n"
	      "    %s *za = (%s *)( m_ + %lu );\n"
	      "    const %s *B = (const %s *)( m_ + %lu );\n"
	      "    %s i;\n",
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[0].i),
	      precision(inst),
	      precision(inst),
	      UL(inst->arg[1].i),
	      type(inst->arg[2].i)) ||
	    P(file,
	      "    for (i=0; i<%lu; ++i) {\n"
	      "      if (0.0 >= B[i]) {\n"
	      "        za[i] = 0.0;\n"
	      "      }\n"
	      "    }\n"
	      "  }\n\n",
	      UL(inst->arg[2].i))) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int inst_lineard(const struct g_ann_program_inst *inst, FILE *file) {
	G_UNUSED(inst);
	G_UNUSED(file);
	G_DEBUG(G_ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return -1;
}

static int inst_softmaxd(const struct g_ann_program_inst *inst, FILE *file) {
	G_UNUSED(inst);
	G_UNUSED(file);
	G_DEBUG(G_ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return -1;
}

static int inst_sigmoidd(const struct g_ann_program_inst *inst, FILE *file) {
	G_UNUSED(inst);
	G_UNUSED(file);
	G_DEBUG(G_ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return -1;
}

static int program(const struct g_ann_program *program, FILE *file) {
	const struct g_ann_program_inst *inst;
	int i;

	for (i=1; i<program->size; ++i) {
		inst = &program->inst[i];
		if (G_ANN_PROGRAM_INST_BATCHLOOP == inst->opc) {
			if (inst_batchloop(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_CLEAR == inst->opc) {
			if (inst_clear(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_COPYX == inst->opc) {
			if (inst_copyx(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_MUL1 == inst->opc) {
			if (inst_mul1(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_MUL2 == inst->opc) {
			if (inst_mul2(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_MUL3 == inst->opc) {
			if (inst_mul3(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_MULS == inst->opc) {
			if (inst_muls(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_ADD == inst->opc) {
			if (inst_add(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_SUBY == inst->opc) {
			if (inst_suby(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_RELU == inst->opc) {
			if (inst_relu(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_LINEAR == inst->opc) {
			if (inst_linear(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_SOFTMAX == inst->opc) {
			if (inst_softmax(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_SIGMOID == inst->opc) {
			if (inst_sigmoid(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_RELUD == inst->opc) {
			if (inst_relud(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_LINEARD == inst->opc) {
			if (inst_lineard(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_SOFTMAXD == inst->opc) {
			if (inst_softmaxd(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else if (G_ANN_PROGRAM_INST_SIGMOIDD == inst->opc) {
			if (inst_sigmoidd(inst, file)) {
				G_DEBUG(0);
				return -1;
			}
		}
		else {
			G_DEBUG(G_ERR_SOFTWARE);
			assert( 0 );
			exit(-1);
		}
	}
	inst = &program->inst[0];
	if (G_ANN_PROGRAM_INST_RET == inst->opc) {
		if (inst_ret(inst, file)) {
			G_DEBUG(0);
			return -1;
		}
	}
	else if (G_ANN_PROGRAM_INST_RETARG == inst->opc) {
		if (inst_retarg(inst, file)) {
			G_DEBUG(0);
			return -1;
		}
	}
	else {
		G_DEBUG(G_ERR_SOFTWARE);
		assert( 0 );
		exit(-1);
	}
	return 0;
}

static int header(const struct g_ann *ann, FILE *file, int includes) {
	time_t t;

	t = time(0);
	if (P(file,
	      "/*\n"
	      " * Auto Generated by The Gravity Compiler - %s"
	      " * Copyright (C) Tony Givargis, 2019\n"
	      " */\n\n",
	      ctime(&t),
	      ann->module)) {
	}
	if (includes) {
		if (P(file,
		      "#include <string.h>\n"
		      "#include <math.h>\n"
		      "#include \"%s.h\"\n\n",
		      ann->module)) {
			G_DEBUG(0);
			return -1;
		}
	}
	return 0;
}

static int activatex(const struct g_ann *ann, FILE *file) {
	const struct g_ann_program *prog;

	prog = &ann->program[G_ANN_PROGRAM_ACTIVATEX];
	if (P(file,
	      "static %s *_activatex_(char *m_, const %s *x_) {\n",
	      precision(&prog->inst[0]),
	      precision(&prog->inst[0])) ||
	    program(prog, file) ||
	    P(file, "}\n\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int activate(const struct g_ann *ann, FILE *file) {
	const struct g_ann_program *prog;

	prog = &ann->program[G_ANN_PROGRAM_ACTIVATE];
	if (P(file,
	      "static %s *_activate_(char *m_, const %s *x_) {\n",
	      precision(&prog->inst[0]),
	      precision(&prog->inst[0])) ||
	    program(prog, file) ||
	    P(file, "}\n\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int backprop(const struct g_ann *ann, FILE *file) {
	const struct g_ann_program *prog;

	prog = &ann->program[G_ANN_PROGRAM_BACKPROP];
	if (P(file,
	      "static void _backprop_(char *m_, const %s *y_) {\n",
	      precision(&prog->inst[0])) ||
	    program(prog, file) ||
	    P(file, "}\n\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int train(const struct g_ann *ann, FILE *file) {
	const struct g_ann_program *prog;

	prog = &ann->program[G_ANN_PROGRAM_TRAIN];
	if (P(file,
	      "static void _train_(char *m_, const %s *x_, const %s *y_) {\n",
	      precision(&prog->inst[0]),
	      precision(&prog->inst[0])) ||
	    program(prog, file) ||
	    P(file, "}\n\n")) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

static int export(const struct g_ann *ann, FILE *file1, FILE *file2) {
	const struct g_ann_program *prog1;
	const struct g_ann_program *prog2;
	const char *prefix;
	uint64_t n1, n2;

	prog1 = &ann->program[G_ANN_PROGRAM_ACTIVATEX];
	prog2 = &ann->program[G_ANN_PROGRAM_TRAIN];
	n1  = ann->memory[G_ANN_MEMORY_ACTIVATE].unit;
	n1 *= ann->memory[G_ANN_MEMORY_ACTIVATE].size;
	n2  = ann->memory[G_ANN_MEMORY_TRAIN].unit;
	n2 *= ann->memory[G_ANN_MEMORY_TRAIN].size;
	prefix = capitalize(ann->prefix);

	/* C file */

	if (P(file1,
	      "%s *%s_activate(char *m_, const %s *x_) {\n"
	      "  return _activatex_(m_, x_);\n"
	      "}\n\n",
	      precision(&prog1->inst[0]),
	      ann->prefix,
	      precision(&prog1->inst[0])) ||
	    P(file1,
	      "void %s_train(char *m_, const %s *x_, const %s *y_) {\n"
	      "  _train_(m_, x_, y_);\n"
	      "}\n\n",
	      ann->prefix,
	      precision(&prog2->inst[0]),
	      precision(&prog2->inst[0]))) {
		G_FREE(prefix);
		G_DEBUG(0);
		return -1;
	}

	/* H file */

	if (P(file2,
	      "#ifndef _%s_H_\n"
	      "#define _%s_H_\n\n"
	      "#define _%s_VERSION %d\n"
	      "#define _%s_ACTIVATE_MEMORY %lu\n"
	      "#define _%s_TRAIN_MEMORY %lu\n\n",
	      prefix,
	      prefix,
	      prefix,
	      G_VERSION,
	      prefix,
	      n1,
	      prefix,
	      n2) ||
	    P(file2,
	      "%s *%s_activate(char *m_, const %s *x_);\n",
	      precision(&prog1->inst[0]),
	      ann->prefix,
	      precision(&prog1->inst[0])) ||
	    P(file2,
	      "void %s_train(char *m_, const %s *x_, const %s *y_);\n\n",
	      ann->prefix,
	      precision(&prog2->inst[0]),
	      precision(&prog2->inst[0])) ||
	    P(file2,
	      "#endif /* _%s_H_ */\n",
	      prefix)) {
		G_FREE(prefix);
		G_DEBUG(0);
		return -1;
	}
	G_FREE(prefix);
	return 0;
}

int g_emitc(const struct g_ann *ann) {
	FILE *file1 = stdout;
	FILE *file2 = stdout;
	char *s;

	assert( ann );

	s = g_malloc(g_strlen(ann->module) + 32);
	if (!s) {
		G_DEBUG(0);
		return -1;
	}
	g_sprintf(s,
		  g_strlen(ann->module) + 32,
		  "%s.c",
		  ann->module);
	file1 = fopen(s, "w");
	g_sprintf(s,
		  g_strlen(ann->module) + 32,
		  "%s.h",
		  ann->module);
	file2 = fopen(s, "w");
	G_FREE(s);
	if (!file1 || !file2) {
		G_DEBUG(0);
		return -1;
	}
	if (header(ann, file1, 1) ||
	    header(ann, file2, 0) ||
	    activatex(ann, file1) ||
	    activate(ann, file1) ||
	    backprop(ann, file1) ||
	    train(ann, file1) ||
	    export(ann, file1, file2)) {
		fclose(file1);
		fclose(file2);
		G_DEBUG(0);
		return -1;
	}
	fclose(file1);
	fclose(file2);
	return 0;
}
