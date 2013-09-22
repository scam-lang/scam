ver = `grep "VERSION =" ../scam.c | sed "s/.* \"//" | sed "s/\";//"`
SERVER = beastie.cs.ua.edu

#ARCH = i386
#OPTS = 

ARCH = amd64
OPTS =

#ARCH = OSX
#OPTS = -m32

#ARCH = OSX_64
#OPTS =

tar		:	
			(cd ..; touch *.c; make OPTS=$(OPTS))
			cp ../scam .
			cp ../*.lib .
			tar cvzf scam-$(ver)-$(ARCH).tgz scam *.lib
			rm scam
			rm *.lib


install	:	tar
			scp scam-$(ver)-$(ARCH).tgz lusth@beastie.cs.ua.edu:/var/www/scam/
