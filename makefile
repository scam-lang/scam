# these, if linux
CC = gcc
OUT = xcheme
# these, for windows
#LREADLINE = -L /home/lusth/mingw32/lib
#IREADLINE = -L /home/lusth/mingw32/include
#CC = i386-mingw32-gcc
#OUT = xcheme.exe

PARSEOBJS = parser.o cell.o lexer.o types.o util.o
OBJS = $(PARSEOBJS) pp.o prim.o env.o eval.o hashtable.o
#PROF = -pg

ALL		: xcheme

xcheme		: $(OBJS) xcheme.o
		$(CC) -o $(OUT) $(PROF) $(LREADLINE) $(OBJS) xcheme.o -lreadline -lhistory -lm # -lcrypt
		cp $(OUT) ~/bin/

parser.o	: parser.c cell.h types.h lexer.h parser.h util.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g parser.c

cell.o		: cell.c cell.h types.h
		$(CC) -c $(PROF) -Wall -g cell.c

env.o		: env.c cell.h types.h env.h
		$(CC) -c $(PROF) -Wall -g env.c

prim.o		: prim.c prim.h
		$(CC) -c $(PROF) -Wall -g prim.c

eval.o		: eval.c cell.h types.h cell.h parser.h env.h eval.h
		$(CC) -c $(PROF) -Wall -g eval.c

lexer.o		: lexer.c cell.h types.h lexer.h  
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g lexer.c

types.o		: types.c types.h
		$(CC) -c $(PROF) -Wall -g types.c

util.o		: util.c util.h types.h cell.h
		$(CC) -c $(PROF) -Wall -g util.c

pp.o		: pp.c types.h
		$(CC) -c $(PROF) -Wall -g pp.c

hashtable.o	: hashtable.c hashtable.h hashtable_private.h
		$(CC) -c $(PROF) -Wall -g hashtable.c

xcheme.o		: xcheme.c cell.h parser.h prim.h eval.h
		$(CC) -c $(PROF) $(IREADLINE) -Wall -g xcheme.c

clean:
		rm *.o xcheme
