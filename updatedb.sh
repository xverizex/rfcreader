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
		rm -r ~/$1/*
	fi
	FIND_NAME=10
	while [ -z "$archive" ]
	do
		archive=$(echo ${UP}|cut -d / -f ${FIND_NAME})
		((FIND_NAME--))
	done
		end=`echo $archive|cut -d . --complement  -f -1`
		case "$end" in 
			"gz") UNPACK=gunzip;;
			"tar.gz") UNPACK="tar zxvf $archive -C $1";;
			"tar.xz") UNPACK="tar xvfp $archive -C $1";;
			"tar.bzip2") UNPACK="tar xvfj $archive -C $1";;
		esac
		${UNPACK}
fi
./rebuild "$1/rfc-index.txt" # list rfc docs
mv index "$1/index"
