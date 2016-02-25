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

public class App: Gtk.Application
{

	public App ()
	{
		Object (application_id: "net.freedesktop.gstreamer.instruments", flags: ApplicationFlags.HANDLES_OPEN);
	}

	protected override void activate ()
	{
		var main_window = new MainWindow (this);
		main_window.show_all ();
		add_window (main_window);
	}


	public override void open (File[] files, string hint)
	{
		foreach (File file in files) {
			var main_window = new MainWindow (this);
			main_window.show_all ();
			string path = file.get_path ();
			stdout.printf ("%s\n", path);
			main_window.open_file (path);
			add_window (main_window);
		}
	}

	public static int main (string[] args)
	{
		App app = new App ();
		return app.run (args);
	}

}
