#ARCH = OSX
#ARCH = i386
ARCH = amd64

install		:	
			(cd ..; touch *.c; make)
			tar cvzf scam-$(ARCH).tgz ../scam ../*.lib
			scp scam-$(ARCH).tgz lusth@beastie.cs.ua.edu:/var/www/scam/
