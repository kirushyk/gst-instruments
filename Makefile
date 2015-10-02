all:
	$(MAKE) -C ./libs/gst/intercept
	$(MAKE) -C ./instruments
	$(MAKE) -C ./ui

install:
	$(MAKE) -C ./libs/gst/intercept install
	$(MAKE) -C ./instruments install
	$(MAKE) -C ./ui install
	$(MAKE) -C ./data install

clean:
	$(MAKE) -C ./libs/gst/intercept clean
	$(MAKE) -C ./instruments clean
	$(MAKE) -C ./ui clean

uninstall:
	$(MAKE) -C ./data uninstall