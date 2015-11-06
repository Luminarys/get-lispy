CC=clang
CFLAGS=-std=c99 -Wall -O2

LIBS=-lm -ledit

MAIN = scli

BINDIR=bin
BINARY=$(BINDIR)/$(MAIN)

IDIR=include
INCLUDES = -I$(IDIR)

SRCDIR = src
SRCS = read.c eval.c print.c loop.c lval.c builtin.c lenv.c mpc.c

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
