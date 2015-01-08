#!/bin/bash
dir=`dirname "$0"`
$dir/../../ucc/bin/as $dir/*.s -l $dir/../../ucc/lib/libucc.s
