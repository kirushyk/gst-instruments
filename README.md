GStreamer Instruments
=====================

GStreamer Instruments is a set of performance analyzing tools for time profiling of GStreamer pipelines and data flow inspection.

![GStreamer Instruments UI](https://pp.vk.me/c631316/v631316037/c00e/OV9WH6sGE6s.jpg)

gst-top
-------

Utiity inspired by top and perf-top, displays performance report for the particular command, analyzing GStreamer ABI calls.

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

	$ DYLD_INSERT_LIBRARIES=/usr/local/lib/libgstintercept.dylib \
		DYLD_FORCE_FLAT_NAMESPACE= \
		GST_DEBUG_DUMP_TRACE_DIR=. \
		gst-play-1.0 ~/Music/Snowman.mp3 --audiosink=fakesink
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

	$ gst-report-1.0 playbin.gsttrace 
	ELEMENT               %CPU   %TIME   TIME
	mad0                   33.4   26.5    806 ms
	source                 10.5    8.3    253 ms
	aconv                   9.8    7.8    237 ms
	streamsynchronizer0     8.4    6.6    202 ms
	mpegaudioparse0         8.2    6.5    197 ms
	typefind                7.1    5.7    172 ms
	inputselector0          7.1    5.6    171 ms
	id3demux0               7.0    5.5    168 ms
	volume                  6.9    5.5    166 ms
	resample                6.7    5.3    161 ms
	playsink                5.9    4.7    142 ms
	aqueue                  5.6    4.5    136 ms
	fakesink0               5.3    4.2    127 ms
	abin                    4.2    3.4    102 ms
	uridecodebin0           0.0    0.0    161 us
	conv                    0.0    0.0   20.0 us
	audiotee                0.0    0.0      0 ns
	identity                0.0    0.0      0 ns
	decodebin0              0.0    0.0      0 ns
	playbin                 0.0    0.0      0 ns
	$

You can generate performance graph in DOT language:
 
	gst-report-1.0 --dot playbin.gsttrace | dot -Tsvg > perf.svg
