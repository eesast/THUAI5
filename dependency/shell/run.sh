#! /bin/bash
cd /usr/local/play
i = 1
while (( $i <= 8))
do
    capi$i
    let "i++"
done