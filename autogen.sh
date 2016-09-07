#!/bin/sh

echo "Checking for autoreconf..."
which "autoreconf" 2>/dev/null || {
  echo "Please install autoconf."
  exit 1
}

echo "Checking for pkg-config..."
which "pkg-config" 2>/dev/null || {
  echo "Please install pkg-config."
  exit 1
}

echo "Checking for aclocal..."
which "aclocal" 2>/dev/null || {
  echo "Please install automake."
  exit 1
}

package=gst-instruments

autoreconf -vif
#./configure $*

./configure $* || {
  echo "Configure failed"
  exit 1
}

echo "Now type 'make' to compile $package."
