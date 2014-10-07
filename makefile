# these, if linux
CC = gcc
OUT = scam
#FLAGS = g3
#FLAGS=-O3
FLAGS = 
OBJS = types.o cell.o lexer.o parser.o prim.o env.o eval.o util.o \
	   thread.o \
	   pp.o pp-base.o \
       sway-lexer.o sway-parser.o sway-pp.o \
       stack.o
OPTS = -O1 # normal runtime
OPTS = -O0 -ggdb -pg -rdynamic# for debugging

LOPTS = 


ALL		: scam

mt	:
	@touch cell.c prim.c

caching : clean
caching : OPTS := -DCACHING $(OPTS)
caching : scam

debug   : clean
debug	: OPTS := -DDEBUG $(OPTS)
debug	: scam

scam		: $(OBJS) scam.o
		$(CC) -o $(OUT) $(LOPTS) $(OPTS) $(OBJS) scam.o -lm -lpthread -lreadline
		cp $(OUT) ~/bin/

install : scam
		sudo mkdir -p /usr/local/lib/scam/
		sudo cp $(OUT) /usr/local/bin
		sudo cp *.lib /usr/local/lib/scam/

parser.o	: parser.c cell.h types.h parser.h util.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) parser.c

stack.o	    : stack.c stack.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) stack.c

cell.o		: cell.c cell.h types.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) cell.c

env.o		: env.c cell.h types.h env.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) env.c

prim.o		: prim.c prim.h types.h cell.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) prim.c

eval.o		: eval.c cell.h types.h cell.h parser.h env.h eval.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) eval.c

lexer.o		: lexer.c cell.h types.h
		$(CC) -c $(OPTS) $(IREADLINE) -Wall $(FLAGS) lexer.c

types.o		: types.c types.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) types.c

util.o		: util.c util.h types.h cell.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) util.c

pp-base.o   : pp-base.c pp-base.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) pp-base.c

pp.o		: pp.c pp.h
		$(CC) -c $(OPTS) -Wall $(FLAGS) pp.c

sway-pp.o	: sway-pp.c
		$(CC) -c $(OPTS) -Wall $(FLAGS) sway-pp.c

scam.o		: scam.c cell.h parser.h prim.h eval.h
		$(CC) -c $(OPTS) $(IREADLINE) -Wall $(FLAGS) scam.c

sway-lexer.o	: sway-lexer.c cell.h types.h parser.h env.h util.h
		$(CC) -c $(OPTS) $(IREADLINE) -Wall $(FLAGS) sway-lexer.c

sway-parser.o	: sway-parser.c cell.h types.h parser.h util.h
		$(CC) -c $(OPTS) $(IREADLINE) -Wall $(FLAGS) sway-parser.c

thread.o	: thread.c thread.h scam.h cell.h pp.h env.h util.h eval.h types.h
		$(CC) -c $(OPTS) $(IREADLINE) -Wall $(FLAGS) thread.c

stacker		: stacker.c scanner.c scanner.h
			gcc -Wall -g -o stacker stacker.c scanner.c

clean:
		rm -f *.o scam gmon.out
		cd testing; make clean
