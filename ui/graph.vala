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

public class Graph: Gtk.DrawingArea
{
	private bool dragging;
	private double x_begin;
	private double x_initial;
	private double x_end;
	public double x_duration;

	public signal void interval_selected (double begin, double end); 

	public void load (string path)
	{
		update ();
	}

	public Graph ()
	{
		add_events (Gdk.EventMask.BUTTON_PRESS_MASK |
		            Gdk.EventMask.BUTTON_RELEASE_MASK |
		            Gdk.EventMask.POINTER_MOTION_MASK);
		
		dragging = false;
		x_begin = x_initial = x_duration = 0;
		x_end = 20000;
		set_size_request (200, 80);
	}

	public override bool draw (Cairo.Context c)
	{
		int i = 0;
		float scale = 1.0f;
		int width = get_allocated_width ();
		int height = get_allocated_height ();

        c.set_source_rgb (1.0, 1.0, 1.0);
		c.rectangle (0, 0, width, height);
		c.fill ();

        c.set_source_rgb (0.0, 0.0, 0.0);

		c.set_line_width (1.0);
		c.move_to (0, height / 2 + 0.5);
		c.line_to (width, height / 2 + 0.5);	
		for (double x = 0.5; x < width; x += 20.0, i++)
		{
			bool separator = (i % 10 == 0);
			c.move_to (x, separator ? 10 : 30);
			c.line_to (x, height - (separator ? 20 : 30));
			if (separator) {
				c.move_to (x + 5, 20);
				c.show_text (@"$(i * 0.1) s");
			}
		}

		c.move_to (x_begin, 0);
		c.line_to (x_begin, height);
		c.move_to (x_end, 0);
		c.line_to (x_end, height);

		c.stroke ();
        c.set_source_rgba (0.0, 0.0, 0.0, 0.25);
		c.rectangle (x_begin, 0, x_end - x_begin, height);
		c.fill ();
        c.set_source_rgba (1.0, 1.0, 1.0, 0.75);
		c.rectangle (x_duration, 0, width - x_duration, height);
		c.fill ();

		return true;
	}

	public override bool button_press_event (Gdk.EventButton event)
	{
		if (!this.dragging) {
			this.dragging = true;
			x_initial = event.x;
		}
		return false;
	}

	public override bool button_release_event (Gdk.EventButton event) {
		if (this.dragging) {
			this.dragging = false;
			this.interval_selected (x_begin / 20.0f, x_end / 20.0f); 
		}
		return false;
        }

	public override bool motion_notify_event (Gdk.EventMotion event)
	{
		if (this.dragging) {
			if (event.x < x_initial) {
				x_begin = event.x;
				x_end = x_initial;
			} else {
				x_begin = x_initial;
				x_end = event.x;
			}
            if (x_begin > x_duration)
				x_begin = x_duration;
            if (x_end > x_duration)
				x_end = x_duration;
			update ();
		}
		return false;
	}

	private void redraw_canvas ()
	{
		var window = get_window ();
		if (window == null)
			return;

		var region = window.get_clip_region ();
		window.invalidate_region (region, true);
		window.process_updates (true);
	}

	public bool update ()
	{
		redraw_canvas ();
		return true;
	}
}

