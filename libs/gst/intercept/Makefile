SRC = intercept.c trace.c
DEPS = gstreamer-1.0
CFLAGS = $(shell pkg-config --cflags $(DEPS)) -I../../../
LIBS = $(shell pkg-config --libs $(DEPS))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SHARED = -dynamiclib
	TARGET = libgstintercept.dylib
endif
ifeq ($(UNAME_S),Linux)
	SHARED = -shared
	TARGET = libgstintercept.so
endif
DEBUG_OPTION = -g

PREFIX = /usr/local/

$(TARGET): $(SRC)
	gcc $(DEBUG_OPTION) $(SHARED) -o $(TARGET) -fPIC -ldl -pthread $(LIBS) $(CFLAGS) $(SRC)

install: $(TARGET)
	cp $(TARGET) $(PREFIX)/lib/

clean:
	rm -rf $(TARGET)
