uname := $(shell uname)

ifneq (,$(findstring NT,$(uname)))
	sl_ext := .dll
endif
ifeq ($(uname),Linux)
	sl_ext := .so
endif
ifeq ($(uname),Darwin)
	sl_ext := .dylib
endif
ifeq (,$(sl_ext))
	sl_ext := .dll
endif

dr-zlib.so: zlib
	$(CC) -O3 -flto -fpic -shared -isystem include -isystem z/include -L z/lib -l z -o dr-zlib$(sl_ext) main.c 

.PHONY: zlib clean
zlib:
	(cd zlib; configure --static -w --prefix "$(CURDIR)/z")
	$(MAKE) -C zlib install

clean:
	$(MAKE) -C zlib distclean
	-rm dr-zlib.so
