SRC=main.vala mainwindow.vala graph.vala mainmenu.vala
TARGET=gst-instruments

VALADEPS=--pkg gtk+-3.0 --pkg posix

$(TARGET): $(SRC)
	valac -o $(TARGET) -g $(VALADEPS) $(SRC) -X -g -X -lm -X -I./

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

clean:
	rm -f $(TARGET)
	rm -f *~

