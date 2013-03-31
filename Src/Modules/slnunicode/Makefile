# put next to lua-5.1.4
# then make -f slnunicode-1.1a/Makefile

unicode.so: slnunicode-1.1a/slnunico.c slnunicode-1.1a/slnudata.c
	gcc -Islnunicode-1.1a -Ilua-5.1.4/src -shared -Wall -Os -fpic -o unicode.so slnunicode-1.1a/slnunico.c
	lua-5.1.4/src/lua slnunicode-1.1a/unitest

clean:
	rm unicode.so
