#!/bin/sh

which "autoreconf" 2>/dev/null || {
  echo "Please install autoconf."
  exit 1
}

which "pkg-config" 2>/dev/null || {
  echo "Please install pkg-config."
  exit 1
}

package=gst-instruments

autoreconf -vif
./configure $*

echo "Now type 'make' to compile $package."
