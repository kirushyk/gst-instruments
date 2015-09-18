public class Graph: Gtk.DrawingArea
{
	public void load(string path)
	{
		update();
	}

	public Graph()
	{
		add_events(Gdk.EventMask.BUTTON_PRESS_MASK |
		           Gdk.EventMask.BUTTON_RELEASE_MASK |
		           Gdk.EventMask.POINTER_MOTION_MASK);
		set_size_request(200, 225);
	}

	public override bool draw(Cairo.Context c)
	{
		float scale = 1.0f;
		int width = get_allocated_width();
		int height = get_allocated_height();

        	c.set_source_rgb(1.0, 1.0, 1.0);
		c.rectangle(0, 0, width, height);
		c.fill();

        	c.set_source_rgb(0.0, 0.0, 0.0);

		double magnitude_scale = 50.0f;

		{
			c.move_to(40.0, 40.0);
			c.show_text("0.5 mV ✕ 100 ms");
		}

		c.set_line_width(0.25);
		for (double x = 0.5; x < width; x += 20.0)
		{
			c.move_to(x, 0);
			c.line_to(x, height);			
		}
		c.stroke();

		return true;
	}

	public override bool button_press_event(Gdk.EventButton event)
	{
		return false;
	}

	public override bool button_release_event(Gdk.EventButton event)
	{
		return false;
	}

	public override bool motion_notify_event(Gdk.EventMotion event)
	{
		return false;
	}

	private void redraw_canvas()
	{
		var window = get_window();
		if (window == null)
		{
			return;
		}

		var region = window.get_clip_region();
		window.invalidate_region(region, true);
		window.process_updates(true);
	}

	private bool update()
	{
		redraw_canvas();
		return true;
	}
}

