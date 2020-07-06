/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
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
	
  public string working_trace_path;
  
  private Gtk.ScrolledWindow scrolled_window;
  private Timeline timeline;

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
    
    scrolled_window = new Gtk.ScrolledWindow (null, null);
    scrolled_window.set_policy (Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
            
    timeline = new Timeline ();
    var scrollbar = new Gtk.Scrollbar (Gtk.Orientation.HORIZONTAL, null);

    working_trace_path = null;

    menu.open_file_item_activated.connect (open_file);
    box.pack_start (scrolled_window, true, true, 1);		

    timeline.interval_selected.connect ((begin, end) =>
    {
      if (working_trace_path == null)
      {
        return;
      }

      var child = scrolled_window.get_child ();
      if (child != null)
      {
        child.destroy ();
      }

      string command = @"gst-report-1.0 --nested --textpads --dot --from=$begin --till=$end $working_trace_path | dot -Tsvg > gst-instruments-temp.svg";
      Posix.system (command);

      var graph = new Gtk.Image.from_file ("gst-instruments-temp.svg");
      scrolled_window.add (graph);
      scrolled_window.show_all ();
    });
    box.pack_start (timeline, false, true, 1);

    scrollbar.value_changed.connect (() =>
    {

    });
    
    box.pack_start (scrollbar, false, true, 1);
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
  
  public void open_file (string path)
  {
    var child = scrolled_window.get_child ();
    if (child != null)
    {
      child.destroy ();
    }
        
    string ls_stdout;
    string ls_stderr;
    int ls_status;

    try
    {
      Process.spawn_sync (null,
                          { "gst-report-1.0", "--duration", path},
                          null,
                          SpawnFlags.SEARCH_PATH,
                          null,
                          out ls_stdout,
                          out ls_stderr,
                          out ls_status);

      timeline.x_duration = int64.parse (ls_stdout) * 0.00000002;
    }
    catch (GLib.SpawnError error	)
    {
    }

    working_trace_path = path;

    string command = @"gst-report-1.0 --nested --textpads --dot $path | dot -Tsvg > gst-instruments-temp.svg";
    Posix.system (command);

    var graph = new Gtk.Image.from_file ("gst-instruments-temp.svg");
    scrolled_window.add (graph);
    scrolled_window.show_all ();
    
    timeline.update ();
  }

}
