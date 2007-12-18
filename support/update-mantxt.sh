#!/bin/sh
#
# update all 00readme.txt and 00readme-ja.txt of the tools from each manuals
#
# should be invoked at parent directory.
#
# If conversion fails, see makeman.sh in this directory.
#

tools_man="\
./adinrec/adinrec.man \
./adintool/adintool.man \
./jcontrol/jcontrol.man \
./mkbingram/mkbingram.man \
./mkbinhmm/mkbinhmm.man \
./mkgshmm/mkgshmm.man \
./mkss/mkss.man \
"

##############################

for m in $tools_man; do
    echo $m
    ./support/makeman.sh $m
    mv $m.txt `dirname $m`/00readme.txt
    mv $m.txt.ja `dirname $m`/00readme-ja.txt
done

echo julius/julius.man
./support/makeman.sh julius/julius.man
mv julius/julius.man.txt julius/00readme.txt
mv julius/julius.man.txt.ja julius/00readme-ja.txt

echo libjulius/jconf.man
./support/makeman.sh libjulius/jconf.man
mv libjulius/jconf.man.txt libjulius/00readme-jconf.txt
mv libjulius/jconf.man.txt.ja libjulius/00readme-jconf-ja.txt

echo Finished.
