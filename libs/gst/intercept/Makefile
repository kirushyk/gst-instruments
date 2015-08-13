SRC = intercept.c
DEPS = gstreamer-1.0
CFLAGS = $(shell pkg-config --cflags $(DEPS))
LIBS = $(shell pkg-config --libs $(DEPS))

libgstintercept.dylib: $(SRC)
	$(CC) -dynamiclib -o libgstintercept.dylib -ldl $(LIBS) $(CFLAGS) $(SRC)

intercept.so: $(SRC)
	gcc -shared -fPIC -ldl -O2 -o intercept.so $(LIBS) $(CFLAGS) $(SRC)

install: libgstintercept.dylib
	cp libgstintercept.dylib /usr/local/lib/

clean:
	rm -rf ./*so*
	rm -rf ./*dylib*
