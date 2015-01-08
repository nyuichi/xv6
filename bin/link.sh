#!/bin/bash
dir=`dirname "$0"`
$dir/../../ucc/bin/as $dir/*.s $dir/../trapasm.S $dir/../swtch.S -l $dir/../../ucc/lib/libucc.s
