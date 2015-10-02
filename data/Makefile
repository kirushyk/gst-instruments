all: pull.svg push.svg

install: pull.svg push.svg
	mkdir -p /usr/local/share/gst-instruments/
	cp pull.svg /usr/local/share/gst-instruments/
	cp push.svg /usr/local/share/gst-instruments/

uninstall:
	rm -f /usr/local/share/gst-instruments/pull.svg
	rm -f /usr/local/share/gst-instruments/push.svg
