#! /bin/sh

if autoreconf -i; then
	rm -rf autom4te.cache
	echo "Now run ./configure"
fi
