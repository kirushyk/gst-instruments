GStreamer Instruments
=====================

gst-top
-------

Inspired by top and perf-top, gst-top displays performance report for the command, analyzing GStreamer ABI calls.

	$ gst-top-1.0 gst-launch-1.0 audiotestsrc num-buffers=1000 ! vorbisenc ! vorbisdec ! fakesink
	Setting pipeline to PAUSED ...
	Pipeline is PREROLLING ...
	Redistribute latency...
	Pipeline is PREROLLED ...
	Setting pipeline to PLAYING ...
	New clock: GstSystemClock
	Got EOS from element "pipeline0".
	Execution ended after 0:00:00.225688000
	Setting pipeline to PAUSED ...
	Setting pipeline to READY ...
	Setting pipeline to NULL ...
	Freeing pipeline ...
	ELEMENT        %CPU   %TIME   TIME
	vorbisenc0      73.8   74.3    172 ms
	vorbisdec0      13.5   13.6   31.4 ms
	audiotestsrc0    8.1    8.2   18.9 ms
	fakesink0        4.0    4.0   9.21 ms
	pipeline0        0.0    0.0      0 ns
	$

libgstintercept
---------------

Intercepts GStreamer ABI calls and records communication between the App and GStreamer into the trace file. 

	$ DYLD_FORCE_FLAT_NAMESPACE= DYLD_INSERT_LIBRARIES=/usr/local/lib/libgstintercept.dylib GST_DEBUG_DUMP_TRACE_DIR=. gst-play-1.0 ~/Music/Snowman.mp3 --audiosink=fakesink
	Press 'k' to see a list of keyboard shortcuts.
	Now playing /Users/cyril/Music/Snowman.mp3
	0:04:29.5 / 0:04:29.5       
	Reached end of play list.
	
	$ ls *.gsttrace
	playbin.gsttrace
	$ 

gst-report
----------

Generates performance report for input trace file.
