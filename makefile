# these, if linux
CC = gcc
OUT = scam

OBJS = types.o cell.o lexer.o parser.o prim.o env.o eval.o util.o pp.o whitespace.o
SOBJS = types.o cell.o lexer.o sparser.o prim.o env.o eval.o util.o pp.o swhitespace.o
#OBJS = types.o cell2.o lexer2.o parser2.o prim.o env.o eval.o util.o pp.o
ROBJS = types.o cell.o lexer.o parser.o runprim.o env.o run.o util.o pp.o whitespace.o
PROF = -pg

ALL		: scam

scam		: $(OBJS) scam.o
		$(CC) -o $(OUT) $(PROF) $(OBJS) scam.o -lm
		cp $(OUT) ~/bin/

install : scam
		sudo mkdir -p /usr/local/lib/scam/
		sudo cp $(OUT) /usr/local/bin
		sudo cp *.lib /usr/local/lib/scam/

sway		: $(SOBJS) sway.o
		$(CC) -o sway2 $(PROF) $(SOBJS) sway.o -lm
		cp sway2 ~/bin/

rscam	: $(ROBJS) scam.o
		$(CC) -o rscam $(PROF) $(ROBJS) scam.o -lm
		cp rscam ~/bin/

parser.o	: parser.c cell.h types.h lexer.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g parser.c
sparser.o	: sparser.c cell.h types.h lexer.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g sparser.c

cell.o		: cell.c cell.h types.h
		$(CC) -c $(PROF) -Wall -g cell.c
#cell2.o		: cell2.c cell.h types.h
#		$(CC) -c $(PROF) -Wall -g cell2.c

env.o		: env.c cell.h types.h env.h
		$(CC) -c $(PROF) -Wall -g env.c

prim.o		: prim.c prim.h types.h cell.h
		$(CC) -c $(PROF) -Wall -g prim.c

eval.o		: eval.c cell.h types.h cell.h parser.h env.h eval.h
		$(CC) -c $(PROF) -Wall -g eval.c

lexer.o		: lexer.c cell.h types.h lexer.h 
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

# analyzed scam specific files

run.o		: run.c cell.h types.h cell.h parser.h env.h run.h
		$(CC) -c $(PROF) -Wall -g run.c

runprim.o		: runprim.c prim.h types.h cell.h
		$(CC) -c $(PROF) -Wall -g runprim.c


clean:
		rm *.o scam
