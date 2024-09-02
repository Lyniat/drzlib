uname := $(shell uname)

arch_name := x86_64
ifneq (,$(findstring NT,$(uname)))
	sl_ext := .dll
endif
ifeq ($(uname),Linux)
	sl_ext := .so
endif
ifeq ($(uname),Darwin)
	sl_ext := .dylib
	extra_flags := -Wl,-undefined,dynamic_lookup
	ifeq ($(macos),x86_64)
		darwin_arch := -target x86_64-apple-macos14.0
	else
		darwin_arch := -target arm64-apple-macos14.0
		arch_name := arm64
	endif
endif
ifeq (,$(sl_ext))
	sl_ext := .dll
endif

dr-zlib$(sl_ext): zlib
	$(CC) -O3 -flto -fpic -shared -isystem include -isystem z/include -L z/lib $(extra_flags) $(darwin_arch) -o dr-zlib-$(arch_name)$(sl_ext) main.c -l z

.PHONY: zlib clean lipo
zlib:
	(cd zlib; CFLAGS="-fpic $(darwin_arch)" ./configure --static -w --prefix "$(CURDIR)/z")
	$(MAKE) -C zlib install

clean:
	$(MAKE) -C zlib distclean
	-rm -f dr-zlib$(sl_ext)
	-rm -f dr-zlib-x86_64$(sl_ext)
	-rm -f dr-zlib-arm64$(sl_ext)

lipo:
	lipo -create -output dr-zlib.dylib dr-zlib-x86_64.dylib dr-zlib-arm64.dylib
