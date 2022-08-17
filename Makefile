apt-roots: apt-roots.cpp
	gcc -g -o $@ $< -lapt-pkg -lstdc++ -std=c++17

install:
	install -D -m 755 apt-roots $(DESTDIR)/usr/bin/apt-roots
	install -D -m 644 apt-roots.1 $(DESTDIR)/usr/share/man/man1/apt-roots.1

indent:
	indent -linux apt-roots.cpp

clean:
	rm -f apt-roots apt-roots.cpp~

.PHONY: install clean
