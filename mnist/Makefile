#
# Makefile
# Copyright (C) Tony Givargis, 2019
#
# The Gravity Compiler
#

CC    = gcc
FLAGS = -ansi -pedantic -Wshadow -Wall -Wextra -Werror -Wfatal-errors -fPIC -O3
LIBS  = ../src/gravity.a -ldl -lm
DEST  = mnist
OBJS  = mnist.o

all: $(OBJS)
	$(CC) -o $(DEST) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(FLAGS) -c $<
	$(CC) $(FLAGS) -MM $< > $*.d

clean:
	rm -f $(DEST) $(OBJS) *.o *.d *~ *#

-include $(OBJS:.o=.d)
