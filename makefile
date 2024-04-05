dr-zlib.so: zlib
	$(CC) -O3 -flto -fpic -shared -isystem include -isystem z/include -L z/lib -l z -o dr-zlib.so main.c 

.PHONY: zlib clean
zlib:
	(cd zlib; configure --static -w --prefix "$(CURDIR)/z")
	$(MAKE) -C zlib install

clean:
	$(MAKE) -C zlib distclean
	-rm dr-zlib.so
