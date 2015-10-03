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

public class MainWindow: Gtk.ApplicationWindow
{

	public MainWindow (Gtk.Application application)
	{
 		Object (application: application, title: "GStreamer Instruments");
		border_width = 0;
		window_position = Gtk.WindowPosition.CENTER;
		set_default_size (800, 600);
		set_icon_name ("utilities-system-monitor");

		var box = new Gtk.Box (Gtk.Orientation.VERTICAL, 0);
		
		var menu = new MainMenu ();
		menu.window = this;
		menu.quit_item_activated.connect (() =>
		{
			this.destroy ();
		});
		// application.set_menubar (menu);
		box.pack_start (menu, false, true, 0);
		
		var scrolled_window = new Gtk.ScrolledWindow (null, null);
		scrolled_window.add_with_viewport (new Gtk.Image.from_file ("/Users/cyril/1.svg"));
		scrolled_window.set_policy (Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
				
		var monitor = new Graph ();
		var scrollbar = new Gtk.Scrollbar (Gtk.Orientation.HORIZONTAL, null);

		menu.open_file_item_activated.connect ((path) => 
		{
			
		});
		box.pack_start (scrolled_window, true, true, 0);
		box.pack_start (monitor, false, true, 0);

		scrollbar.value_changed.connect (() =>
		{
        });
		
		box.pack_start (scrollbar, false, true, 0);
		this.key_press_event.connect ((source, key) => 
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

		add (box);
	}

}

