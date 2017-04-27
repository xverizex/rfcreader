#!/bin/bash
# Запустить с параметром, в котором указан каталог, куда
# распаковать файлы
if [ -z $1 ]
then
	exit 0
fi
UP=https://www.rfc-editor.org/in-notes/tar/RFC-all.tar.gz
UNPACK="w"
if [ -d "$1" ]
then
	wget ${UP} 
	if [ $? = "0" ]
	then
		rm -r $1/*
	fi
	echo "extracting ..."
	tar zxf RFC-all.tar.gz $1
fi
./rebuild "$1/rfc-index.txt" # list rfc docs
if [ -f index ]
then
	mv index "$1/index"
else
	rebuild "$1/rfc-index.txt" # list rfc docs
	mv index "$1/index"
fi
