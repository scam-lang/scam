SERVER=beastie.cs.ua.edu
#SERVER=nixie.cs.ua.edu

ver=0.1.8
arch=i386
#arch=amd64
base=scam-$(ver)_$(arch)

$(base).deb	: 
	rm -f -r debian
	mkdir -p debian
	mkdir -p debian/DEBIAN
	mkdir -p debian/usr/bin/
	mkdir -p debian/usr/lib/scam
	mkdir -p debian/usr/share/doc/scam/
	mkdir -p debian/usr/share/man/man1/
	echo Package: scam > debian/DEBIAN/control
	echo Version: $(ver) >> debian/DEBIAN/control
	echo Architecture: $(arch) >> debian/DEBIAN/control
	cat files/control.rest >> debian/DEBIAN/control
	cp files/postinst debian/DEBIAN/
	chmod 755 debian/DEBIAN/postinst
	cp files/copyright debian/usr/share/doc/scam/
	cp man/scam.1 debian/usr/share/man/man1/
	rm -f debian/usr/share/man/man1/scam.1.gz
	gzip --best debian/usr/share/man/man1/scam.1
	cp files/changelog debian/usr/share/doc/scam/
	rm -f debian/usr/share/doc/scam/changelog.gz
	gzip --best debian/usr/share/doc/scam/changelog
	cp files/changelog.Debian debian/usr/share/doc/scam/
	rm -f debian/usr/share/doc/scam/changelog.Debian.gz
	gzip --best debian/usr/share/doc/scam/changelog.Debian
	(cd ..; touch *.c; make;)
	cp ../scam debian/usr/bin
	cp ../*.lib debian/usr/lib/scam/

	fakeroot dpkg-deb --build debian $(base).deb

install	: $(base).deb
	scp $(base).deb $(SERVER):/var/www/scam/

local	: $(base).deb
	cp $(base).deb /var/www/scam/

clean	:
	@rm -f -r debian
	@rm -f $(base).deb
	@rm -f -r $(base)
