#!/bin/sh -e

aclocal
autoheader
autoconf
automake --add --copy
./configure $@
