uname := $(shell uname)

ifneq (,$(findstring NT,$(uname)))
	sl_ext := .dll
	extra_flags := --target=x86_64-w64-mingw
endif
ifeq ($(uname),Linux)
	sl_ext := .so
endif
ifeq ($(uname),Darwin)
	sl_ext := .dylib
	extra_flags := -Wl,-undefined,dynamic_lookup
endif
ifeq (,$(sl_ext))
	sl_ext := .dll
endif

dr-zlib$(sl_ext): zlib
	[ -d include ] || mkdir include
	cp endian.h include/endian.h
	$(CC) -O3 -flto -fpic -shared -isystem include -isystem z/include -L z/lib -l z $(extra_flags) -o dr-zlib$(sl_ext) main.c 

.PHONY: zlib clean
zlib:
	(cd zlib; ./configure --static -w --prefix "$(CURDIR)/z")
	$(MAKE) -C zlib install

clean:
	$(MAKE) -C zlib distclean
	-rm dr-zlib$(sl_ext)
