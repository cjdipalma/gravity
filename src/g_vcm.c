/**
 * g_vcm.c
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

#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include "g_common.h"
#include "g_vcm.h"

struct g_vcm {
	void *handle;
};

static int compile(const char *input, const char *output) {
	char *file, *argv[15];
	int status;
	pid_t pid;

	pid = fork();
	if (0 > pid) {
		G_DEBUG(G_ERR_SYSTEM);
		return -1;
	}
	if (!pid) {
		file = g_strlen(getenv("CC")) ? getenv("CC") : "/usr/bin/cc";
		argv[ 0] = "cc";
		argv[ 1] = "-ansi";
		argv[ 2] = "-pedantic";
		argv[ 3] = "-Wshadow";
		argv[ 4] = "-Wall";
		argv[ 5] = "-Wextra";
		argv[ 6] = "-Werror";
		argv[ 7] = "-Wfatal-errors";
		argv[ 8] = "-fPIC";
		argv[ 9] = "-O3";
		argv[10] = "-shared";
		argv[11] = (char *)input;
		argv[12] = "-o";
		argv[13] = (char *)output;
		argv[14] = 0;
		if (0 > execvp(file, argv)) {
			G_DEBUG(G_ERR_SYSTEM);
			return -1;
		}
	}
	else {
		status = 0;
		while (pid != waitpid(pid, &status, 0));
		if (status) {
			G_DEBUG(G_ERR_JITC);
			return -1;
		}
	}
	return 0;
}

g_vcm_t g_vcm_open(const char *pathname) {
	const char *output;
	struct g_vcm *vcm;

	assert( g_strlen(pathname) );

	output = g_pathname_open(".so");
	if (!output || compile(pathname, output)) {
		G_FREE(output);
		G_DEBUG(0);
		return 0;
	}
	vcm = g_malloc(sizeof (struct g_vcm));
	if (!vcm) {
		g_pathname_close(output);
		G_DEBUG(0);
		return 0;
	}
	memset(vcm, 0, sizeof (struct g_vcm));
	vcm->handle = dlopen(output, RTLD_LAZY | RTLD_LOCAL);
	g_pathname_close(output);
	if (!vcm->handle) {
		g_vcm_close(vcm);
		G_DEBUG(G_ERR_SYSTEM);
		return 0;
	}
	return vcm;
}

void g_vcm_close(g_vcm_t vcm) {
	if (vcm && vcm->handle) {
		dlclose(vcm->handle);
	}
	G_FREE(vcm);
}

long g_vcm_lookup(g_vcm_t vcm, const char *symbol) {
	assert( vcm && g_strlen(symbol) );

	return (long)dlsym(vcm->handle, symbol);
}
