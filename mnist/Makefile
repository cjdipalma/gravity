#
# Makefile
# Copyright (C) Tony Givargis, 2019-2020
#
# This file is part of The Gravity Compiler.
#
# The Gravity Compiler is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version. The Gravity Compiler is distributed in
# the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE. See the GNU General Public License for more details. You should
# have received a copy of the GNU General Public License along with Foobar.
# If not, see <https://www.gnu.org/licenses/>.
#

CC    = gcc
FLAGS = -ansi -pedantic -Wshadow -Wall -Wextra -Werror -Wfatal-errors -fPIC -O3
LIBS  = ../src/libgravity.a ../src/test.h -ldl -lm
DEST  = mnist
OBJS  = mnist.o
IDK   = mnist-static.c ../src/test.c avr-sim.c dump-ram.c
IDK2  = mnist-avr.c test-avr.c avr-sim.c dump-ram.c

all: $(OBJS)
	$(CC) -o $(DEST) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(FLAGS) -c $<
	$(CC) $(FLAGS) -MM $< > $*.d

clean:
	rm -f $(DEST) $(OBJS) *.o *.d *~ *#

static:
	$(CC) $(IDK) -o $(DEST) -lm

avr:
	$(CC) $(IDK2) -o $(DEST) -lm

-include $(OBJS:.o=.d)
