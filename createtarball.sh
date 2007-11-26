#!/bin/sh

svn_revision=`LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2`
test $svn_revision || svn_revision=`grep revision .svn/entries 2>/dev/null | cut -d '"' -f2`
test $svn_revision || svn_revision=`sed -n -e '/^dir$/{n;p;q;}' .svn/entries 2>/dev/null`
test $svn_revision || svn_revision=UNKNOWN


name="instlux-SVN${svn_revision}"
mkdir $name
cp -Rv grub4dos $name/
cp -Rv instlux $name/
cp -Rv README.grub4dos $name/
find $name -name .svn -print0 | xargs -0 rm -rf
rm -rf $name/instlux/distros/*
tar -cvjf $name.tar.bz2 $name



