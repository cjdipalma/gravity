/**
 * g_vcm.h
 * Copyright (C) Tony Givargis, 2019-2020
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

#ifndef _G_VCM_H_
#define _G_VCM_H_

typedef struct g__vcm *g__vcm_t;

g__vcm_t g__vcm_open(const char *pathname);

void g__vcm_close(g__vcm_t vcm);

long g__vcm_lookup(g__vcm_t vcm, const char *symbol);

#endif /* _G_VCM_H_ */
