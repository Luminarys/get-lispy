CC=clang

CFLAGS=-std=c99 -Wall -g

LIBS=-lm -ledit

MAIN = gl

BINDIR=bin
BINARY=$(BINDIR)/$(MAIN)

IDIR=include
INCLUDES = -I$(IDIR)

SRCDIR = src
SRCS = read.c eval.c print.c loop.c lval.c builtin.c lenv.c mpc.c log.c vm.c list.c

OBJDIR = obj
OBJS = $(SRCS:.c=.o)
OBJFILES = $(addprefix $(OBJDIR)/, $(OBJS))

.PHONY: clean

all: $(MAIN)
	@echo Compiled $(MAIN)!

$(MAIN): $(OBJFILES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BINARY) $(OBJFILES) $(LIBS)

$(OBJDIR)/%.o:$(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) $(BINDIR)/*
