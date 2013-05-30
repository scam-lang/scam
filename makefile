# these, if linux
CC = gcc
OUT = scam

OBJS = types.o cell.o lexer.o parser.o prim.o env.o eval.o util.o pp.o \
       sway-lexer.o sway-parser.o sway-pp.c
PROF = 
PROF = -pg

ALL		: scam

scam		: $(OBJS) scam.o
		$(CC) -o $(OUT) $(PROF) $(OBJS) scam.o -lm
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

scam.o		: scam.c cell.h parser.h prim.h eval.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g scam.c

whitespace.o : whitespace.c cell.h parser.h
		$(CC) -c $(PROF) -Wall -g whitespace.c

swhitespace.o : swhitespace.c cell.h parser.h
		$(CC) -c $(PROF) -Wall -g swhitespace.c

sway-parser.o	: sway-parser.c cell.h types.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g sway-parser.c

clean:
		rm *.o scam
