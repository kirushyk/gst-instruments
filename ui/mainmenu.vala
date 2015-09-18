public class MainMenu: Gtk.MenuBar
{
	
	public signal void quit_item_activated();

	public signal void open_file_item_activated(string path); 

	public signal void limb_leads_item_activated();
	public signal void augment_limb_leads_item_activated();
	public signal void chest_leads_item_activated();

	public MainMenu()
	{
		var file_item = new Gtk.MenuItem.with_label("File");

		var view_item = new Gtk.MenuItem.with_label("View");
		var view_menu = new Gtk.Menu();
		view_item.set_submenu(view_menu);
		var limb_leads_item = new Gtk.MenuItem.with_label("Limb Leads");
		view_menu.add(limb_leads_item);
		limb_leads_item.activate.connect(() => {limb_leads_item_activated();});
		var augment_limb_leads_item = new Gtk.MenuItem.with_label("Augment Limb Leads");
		view_menu.add(augment_limb_leads_item);
		augment_limb_leads_item.activate.connect(() => {augment_limb_leads_item_activated();});
		var chest_leads_item = new Gtk.MenuItem.with_label("Chest Leads");
		view_menu.add(chest_leads_item);
		chest_leads_item.activate.connect(() => {chest_leads_item_activated();});

		var help_item = new Gtk.MenuItem.with_label("Help");
		var help_menu = new Gtk.Menu();
		help_item.set_submenu(help_menu);
		var about_item = new Gtk.MenuItem.with_label("About");
		help_menu.add(about_item);
		about_item.activate.connect(() =>
		{
			var dialog = new Gtk.MessageDialog(null, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, "GStreamer Instruments");
			dialog.run();
			dialog.destroy();
		});

		var file_menu = new Gtk.Menu();
		file_item.set_submenu(file_menu);

		var open_item = new Gtk.MenuItem.with_label("Open");
		file_menu.add(open_item);
		open_item.activate.connect(() =>
		{
        		var file_chooser = new Gtk.FileChooserDialog("Open File", null, Gtk.FileChooserAction.OPEN, Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.OPEN, Gtk.ResponseType.ACCEPT);
			var filter = new Gtk.FileFilter();
			filter.set_filter_name("Gstreamer Trace (*.gsttrace)");
			filter.add_pattern("*.gsttrace");
			filter.add_pattern("*.gsttrace");
			file_chooser.add_filter(filter);
        		if (file_chooser.run() == Gtk.ResponseType.ACCEPT)
			{
            			this.open_file_item_activated(file_chooser.get_filename());
        		}
        		file_chooser.destroy();
		});
		var quit_item = new Gtk.MenuItem.with_label("Quit");
		quit_item.activate.connect(() => {quit_item_activated();});
		file_menu.add(quit_item);

		append(file_item);
		append(view_item);
		append(help_item);
	}

}

