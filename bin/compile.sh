#!/bin/bash
dir=`dirname "$0"`
for f in $dir/../*.c
do
    x=${f##*/}
    echo $x;
    $dir/../../ucc/bin/ucc -s -o $dir/${x%.*}.s $f
done

rm $dir/ide.s
rm $dir/lapic.s
