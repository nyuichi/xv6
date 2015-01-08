#!/bin/bash
dir=`dirname "$0"`
rm -f $dir/*.out*
$dir/../../ucc/bin/as  -o $dir/xv6.out $dir/*.s $dir/../trapasm.S $dir/../swtch.S -l $dir/../../ucc/lib/libucc.s -s
