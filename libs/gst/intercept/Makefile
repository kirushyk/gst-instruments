SRC = intercept.c
DEPS = gstreamer-1.0
CFLAGS = $(shell pkg-config --cflags $(DEPS))
LIBS = $(shell pkg-config --libs $(DEPS))

libgstintercept.dylib: $(SRC)
	$(CC) -dynamiclib -o libgstintercept.dylib -fPIC -ldl $(LIBS) $(CFLAGS) $(SRC)

libgstintercept.so: $(SRC)
	gcc -shared -o libgstintercept.so -fPIC -ldl -pthread $(LIBS) $(CFLAGS) $(SRC)

install: libgstintercept.dylib
	cp libgstintercept.dylib /usr/local/lib/

install-so: libgstintercept.so
	cp libgstintercept.so /usr/local/lib/

clean:
	rm -rf ./*so*
	rm -rf ./*dylib*
