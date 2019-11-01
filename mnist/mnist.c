/**
 * mnist.c
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

#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../src/g.h"

#define UL(x) ((unsigned long)(x))

#define BATCH  8  /* should match .batch specification */
#define EPOCHS 4  /* number of training episodes */

typedef float real_t; /* should match .precision specification */

static g_t g;

static uint64_t usec(void) {
	struct timeval t;

	gettimeofday(&t, 0);
	return (uint64_t)t.tv_usec + (uint64_t)t.tv_sec * 1000000;
}

static int32_t swap(int32_t x) {
	union { int32_t i; char b[4]; } in, out;

	in.i = x;
	out.b[0] = in.b[3];
	out.b[1] = in.b[2];
	out.b[2] = in.b[1];
	out.b[3] = in.b[0];
	return out.i;
}

static int argmax(const real_t *a, int n) {
	real_t max;
	int i, j;

	max = a[0];
	for (i=j=0; i<n; ++i) {
		if (max < a[i]) {
			max = a[i];
			j = i;
		}
	}
	return j;
}

static int train_and_test(const uint8_t *train_labels,
			  const uint8_t *train_images,
			  const uint8_t *test_labels,
			  const uint8_t *test_images,
			  int train_n,
			  int test_n) {
	const uint8_t *labels, *images;
	int i, j, k, m, error;
	real_t *x, *y, *z;
	uint64_t t;

	x = (real_t *)malloc(BATCH * 28 * 28 * sizeof (x[0]));
	y = (real_t *)malloc(BATCH * 10 * sizeof (y[0]));
	if (!x || !y) {
		free(x);
		free(y);
		printf("out of memory\n");
		return -1;
	}

	/* train */

	t = usec();
	m = train_n / BATCH;
	labels = train_labels;
	images = train_images;
	for (i=0; i<m; ++i) {
		for (j=0; j<BATCH; ++j) {
			for (k=0; k<(28*28); ++k) {
				x[j * (28*28) + k] = (*images++) / 255.0;
			}
			for (k=0; k<10; ++k) {
				y[j * 10 + k] = 0.0;
			}
			y[j * 10 + (*labels++)] = 1.0;
		}
		g_train(g, x, y);
		printf("\r%06d/%06d", i, m);
		fflush(stdout);
	}
	printf("\rtrain done (%.2f sec)\n", (usec() - t) * 1e-6);

	/* test */

	t = usec();
	error = 0;
	labels = test_labels;
	images = test_images;
	for (i=0; i<test_n; ++i) {
		for (k=0; k<(28*28); ++k) {
			x[k] = (*images++) / 255.0;
		}
		z = g_activate(g, x);
		if (argmax(z, 10) != (int)(*labels++)) {
			error++;
		}
		printf("\r%06d/%06d", i, test_n);
		fflush(stdout);
	}
	printf("\rtest done (%.2f sec, %d errors)\n", (usec()-t)*1e-6, error);

	/* done */

	free(x);
	free(y);
	return 0;
}

static uint8_t *load_labels(const char *pathname, int *n) {
	int32_t meta[2];
	uint8_t *data;
	FILE *file;

	file = fopen(pathname, "r");
	if (!file) {
		printf("unable to open file\n");
		return 0;
	}
	if (sizeof (meta) != fread(meta, 1, sizeof (meta), file)) {
		fclose(file);
		printf("unable to read file\n");
		return 0;
	}
	if ((0x1080000 != meta[0]) || (0 >= swap(meta[1]))) {
		fclose(file);
		printf("invalid file\n");
		return 0;
	}
	(*n) = swap(meta[1]);
	meta[1] = (*n);
	data = (uint8_t *)malloc(meta[1]);
	if (!data) {
		fclose(file);
		printf("out of memory\n");
		return 0;
	}
	if ((size_t)meta[1] != fread(data, 1, meta[1], file)) {
		free(data);
		fclose(file);
		printf("unable to read file\n");
		return 0;
	}
	fclose(file);
	return data;
}

static uint8_t *load_images(const char *pathname, int *n) {
	int32_t meta[4];
	uint8_t *data;
	FILE *file;

	file = fopen(pathname, "r");
	if (!file) {
		printf("unable to open file\n");
		return 0;
	}
	if (sizeof (meta) != fread(meta, 1, sizeof (meta), file)) {
		fclose(file);
		printf("unable to read file\n");
		return 0;
	}
	if ((0x3080000 != meta[0]) ||
	    (0  >= swap(meta[1])) ||
	    (28 != swap(meta[2])) ||
	    (28 != swap(meta[3]))) {
		fclose(file);
		printf("invalid file\n");
		return 0;
	}
	(*n) = swap(meta[1]);
	meta[1] = (*n) * 28 * 28;
	data = (uint8_t *)malloc(meta[1]);
	if (!data) {
		fclose(file);
		printf("out of memory\n");
		return 0;
	}
	if ((size_t)meta[1] != fread(data, 1, meta[1], file)) {
		free(data);
		fclose(file);
		printf("unable to read file\n");
		return 0;
	}
	fclose(file);
	return data;
}

int main() {
	int train_labels_n, train_images_n, test_labels_n, test_images_n;
	uint8_t *train_labels, *train_images, *test_labels, *test_images;
	int i, e;

	/* open ANN */

	g_debug(1);
	g = g_open(".precision float",
		   ".costfnc cross_entropy",
		   ".batch 8",
		   ".eta 0.1",
		   ".input 28 * 28",
		   ".output 10 softmax",
		   ".hidden 100 relu",
		   ".hidden 100 relu",
		   0);
	if (!g) {
		printf("g_open error\n");
		return -1;
	}
	printf("version: %d\n", g_version());
	printf("size   : %lu\n", UL(g_memory_size(g)));
	printf("hard   : %lu\n", UL(g_memory_hard(g)));

	/* load train/test data */

	e = 0;
	train_labels = load_labels("data/train-labels", &train_labels_n);
	train_images = load_images("data/train-images", &train_images_n);
	test_labels = load_labels("data/test-labels", &test_labels_n);
	test_images = load_images("data/test-images", &test_images_n);
	if (!train_labels ||
	    !train_images ||
	    !test_labels ||
	    !test_images ||
	    (train_labels_n != train_images_n) ||
	    (test_labels_n != test_images_n)) {
		e = -1;
		printf("failed to load valid train/test data\n");
	}

	/* train and test */

	for (i=0; i<EPOCHS; ++i) {
		printf("--- EPOCH %d ---\n", i);
		if (!e) {
			if (train_and_test(train_labels,
					   train_images,
					   test_labels,
					   test_images,
					   train_labels_n,
					   test_labels_n)) {
				e = -1;
				printf("failed to train/test\n");
			}
		}
	}

	/* close */

	free(train_labels);
	free(train_images);
	free(test_labels);
	free(test_images);
	g_close(g);
	return e;
}
