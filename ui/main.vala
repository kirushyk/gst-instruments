public class App: Gtk.Application
{

	public App()
	{
		Object(application_id: "org.gstreamer.instruments", flags: ApplicationFlags.FLAGS_NONE);
	}

	protected override void activate()
	{
		Gtk.Window window = new MainWindow(this);
		window.show_all();
		add_window(window);
	}

	public static int main(string[] args)
	{
		App app = new App();
		return app.run(args);
	}

}

