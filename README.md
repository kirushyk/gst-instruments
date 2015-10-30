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

gst-report
----------

Generates performance report for input trace file.
