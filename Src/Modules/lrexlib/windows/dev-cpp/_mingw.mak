# tested with GNU Make

LIBS      = --add-stdcall-alias $(MYLIBS) -s
INCS      = $(MYINCS)
BIN       = $(PROJECT).dll
DEFFILE   = $(PROJECT).def
BININSTALL= $(INSTALLPATH)\$(BIN)
CC        = gcc.exe
CFLAGS    = $(INCS) -DREX_OPENLIB=luaopen_$(PROJECT) \
  -DREX_LIBNAME=\"$(PROJECT)\" $(MYCFLAGS)

.PHONY: all install test clean

vpath %.c $(SRCPATH)
vpath %.h $(SRCPATH)

all: $(BIN)

clean:
	del $(OBJ) $(BIN) $(DEFFILE)

install: $(BININSTALL)

test:
	cd $(TESTPATH) && lua runtest.lua $(TESTNAME)

$(BIN): $(OBJ) $(DEFFILE)
	$(CC) $(DEFFILE) $(OBJ) $(LIBS) -o $@ -shared

$(DEFFILE):
	lua -e"print('EXPORTS') for k,v in ipairs{$(EXPORTED)} do \
      print('\t'..v) end" > $@

$(BININSTALL): $(BIN)
	copy /Y $< $@

