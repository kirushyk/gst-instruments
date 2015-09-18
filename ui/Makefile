SRC=main.vala mainwindow.vala graph.vala mainmenu.vala
TARGET=gst-instruments

$(TARGET): $(SRC)
	valac -o $(TARGET) -g --pkg gtk+-3.0 $(SRC) -X -g -X -lm -X -I./

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

clean:
	rm -f $(TARGET)
	rm -f *~

