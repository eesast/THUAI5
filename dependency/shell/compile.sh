#! /bin/bash
i=1
while (( $i<= 8 ))
do
    mv -f /usr/local/mnt/player$i.cpp ./API/src/AI.cpp
    cmake CMakeLists.txt && make
    mv capi /usr/local/play/capi$i
    let "i++"
done