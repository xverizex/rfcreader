#!/bin/sh
aclocal
if [ $? != 0 ]; then exit;fi
autoheader
if [ $? != 0 ]; then exit;fi
autoconf
if [ $? != 0 ]; then exit;fi
automake -a -c --add-missing
if [ $? != 0 ]; then exit;fi
