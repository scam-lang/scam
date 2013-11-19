# these, if linux
CC = gcc
OUT = scam
OPTS = 

OBJS = types.o cell.o lexer.o parser.o prim.o env.o eval.o util.o pp.o \
       sway-lexer.o sway-parser.o nsem.o
	   #sem.o
PROF = 
PROF = -pg

ALL		: scam

scam		: $(OBJS) scam.o
		$(CC) -o $(OUT) $(OPTS) $(PROF) $(OBJS) scam.o -lm -pthread
		cp $(OUT) ~/bin/

install : scam
		sudo mkdir -p /usr/local/lib/scam/
		sudo cp $(OUT) /usr/local/bin
		sudo cp *.lib /usr/local/lib/scam/

parser.o	: parser.c cell.h types.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g parser.c

cell.o		: cell.c cell.h types.h
		$(CC) -c $(PROF) -Wall -g cell.c

env.o		: env.c cell.h types.h env.h
		$(CC) -c $(PROF) -Wall -g env.c

prim.o		: prim.c prim.h types.h cell.h
		$(CC) -c $(PROF) -Wall -g prim.c

eval.o		: eval.c cell.h types.h cell.h parser.h env.h eval.h
		$(CC) -c $(PROF) -Wall -g eval.c

lexer.o		: lexer.c cell.h types.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g lexer.c

types.o		: types.c types.h
		$(CC) -c $(PROF) -Wall -g types.c

util.o		: util.c util.h types.h cell.h
		$(CC) -c $(PROF) -Wall -g util.c

pp.o		: pp.c pp.h
		$(CC) -c $(PROF) -Wall -g pp.c

nsem.o		: nsem.c cell.h types.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g nsem.c

sem.o		: sem.c cell.h types.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g sem.c

scam.o		: scam.c cell.h parser.h prim.h eval.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g scam.c

sway-lexer.o	: sway-lexer.c cell.h types.h parser.h env.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g sway-lexer.c

sway-parser.o	: sway-parser.c cell.h types.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g sway-parser.c

clean:
		rm *.o scam
