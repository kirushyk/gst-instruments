public class MainWindow: Gtk.ApplicationWindow
{

	public MainWindow(Gtk.Application application)
	{
 		Object(application: application, title: "GStreamer Instruments");
		border_width = 0;
		window_position = Gtk.WindowPosition.CENTER;
		set_default_size(800, 600);
		set_icon_name("utilities-system-monitor");

		var box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		
		var menu = new MainMenu();
		menu.quit_item_activated.connect(() =>
		{
			this.destroy();
		});
		// application.set_menubar(menu);
		box.pack_start(menu, false, true, 0);

		var monitor = new Graph();

		var scrollbar = new Gtk.Scrollbar(Gtk.Orientation.HORIZONTAL, null);

		menu.open_file_item_activated.connect((path) => 
		{
		});
		box.pack_start(monitor, true, true, 0);

		scrollbar.value_changed.connect(() =>
		{
        	});
		box.pack_start(scrollbar, false, true, 0);
		this.key_press_event.connect((source, key) => 
		{

			switch (key.keyval)
			{
			case Gdk.Key.Left:
			case Gdk.Key.leftarrow:
				break;
			case Gdk.Key.Right:
			case Gdk.Key.rightarrow:
				break;
			case Gdk.Key.space:
				break;
			default:
				break;
			}
			
			return true;
		});

		add(box);
	}

}

