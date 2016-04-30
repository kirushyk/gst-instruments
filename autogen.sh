#!/bin/sh

package=gst-instruments

autoreconf -vif
./configure $*

echo "Now type 'make' to compile $package."
