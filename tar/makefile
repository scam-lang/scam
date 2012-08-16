ver = `grep "VERSION =" ../scam.c | sed "s/.* \"//" | sed "s/\";//"`
SERVER = beastie.cs.ua.edu
ARCH = i386
#ARCH = amd64
#ARCH = OSX

tar		:	
			(cd ..; touch *.c; make)
			tar cvzf scam-$(ver)-$(ARCH).tgz ../scam ../*.lib

install	:	tar
			scp scam-$(ver)-$(ARCH).tgz lusth@beastie.cs.ua.edu:/var/www/scam/
