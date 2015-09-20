/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify i t under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

public class MainMenu: Gtk.MenuBar
{
	public Gtk.ApplicationWindow window;

	public signal void quit_item_activated ();

	public signal void open_file_item_activated (string path); 

	public signal void limb_leads_item_activated ();
	public signal void augment_limb_leads_item_activated ();

	public MainMenu()
	{
		window = null;
		var file_item = new Gtk.MenuItem.with_label ("File");

		var view_item = new Gtk.MenuItem.with_label ("View");
		var view_menu = new Gtk.Menu ();
		view_item.set_submenu (view_menu);
		var limb_leads_item = new Gtk.MenuItem.with_label ("Memory Usage");
		view_menu.add (limb_leads_item);
		limb_leads_item.activate.connect (() => { limb_leads_item_activated (); });
		var augment_limb_leads_item = new Gtk.MenuItem.with_label ("CPU Usage");
		view_menu.add (augment_limb_leads_item);
		augment_limb_leads_item.activate.connect (() => { augment_limb_leads_item_activated (); });

		var help_item = new Gtk.MenuItem.with_label ("Help");
		var help_menu = new Gtk.Menu ();
		help_item.set_submenu (help_menu);
		var about_item = new Gtk.MenuItem.with_label ("About");
		help_menu.add (about_item);
		about_item.activate.connect (() =>
		{
			var dialog = new Gtk.MessageDialog (window, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, "GStreamer Instruments\nCopyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>");
			dialog.run ();
			dialog.destroy ();
		});

		var file_menu = new Gtk.Menu ();
		file_item.set_submenu (file_menu);

		var open_item = new Gtk.MenuItem.with_label ("Open");
		file_menu.add (open_item);
		open_item.activate.connect (() =>
		{
        		var file_chooser = new Gtk.FileChooserDialog ("Open File", null, Gtk.FileChooserAction.OPEN, Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.OPEN, Gtk.ResponseType.ACCEPT);
			var filter = new Gtk.FileFilter ();
			filter.set_filter_name ("GStreamer Trace (*.gsttrace)");
			filter.add_pattern ("*.gsttrace");
			file_chooser.add_filter (filter);
        		if (file_chooser.run () == Gtk.ResponseType.ACCEPT)
			{
            			this.open_file_item_activated (file_chooser.get_filename ());
        		}
        		file_chooser.destroy ();
		});
		var quit_item = new Gtk.MenuItem.with_label ("Quit");
		quit_item.activate.connect (() => { quit_item_activated(); });
		file_menu.add (quit_item);

		append (file_item);
		append (view_item);
		append (help_item);
	}

}

