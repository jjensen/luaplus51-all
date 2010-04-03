# Makefile for lrexlib

# See src/*.mak for user-definable settings

POSIX = src/posix
PCRE = src/pcre
ONIG = src/oniguruma

all: build test

build: build_pcre build_posix build_onig

test: test_pcre test_posix test_onig

clean: clean_pcre clean_posix clean_onig

build_pcre:
	make -C $(PCRE) -f rex_pcre.mak

build_posix:
	make -C $(POSIX) -f rex_posix.mak

build_onig:
	make -C $(ONIG) -f rex_onig.mak

test_pcre:
	cd test && lua ./runtest.lua -d../$(PCRE) pcre

test_posix:
	cd test && lua ./runtest.lua -d../$(POSIX) posix

test_onig:
	cd test && lua ./runtest.lua -d../$(ONIG) onig

clean_pcre:
	make -C $(PCRE) -f rex_pcre.mak clean

clean_posix:
	make -C $(POSIX) -f rex_posix.mak clean

clean_onig:
	make -C $(ONIG) -f rex_onig.mak clean

.PHONY: all build test clean build_pcre test_pcre clean_pcre build_posix \
  test_posix clean_posix build_onig test_onig clean_onig
