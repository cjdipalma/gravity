/*
 * Auto Generated by The Gravity Compiler - Fri Jun 26 02:13:00 2020
 * Copyright (C) Tony Givargis, 2019-2020
 */

#ifndef _TEST_H_
#define _TEST_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int test_version(void);
size_t test_memory_size(void);
size_t test_memory_hard(void);
void test_initialize();
void *test_activate(const void *x);
void test_train(const void *x, const void *y);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TEST_H_ */
