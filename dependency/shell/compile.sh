#! /bin/bash 
# WORKDIR /usr/local/
cd ./CAPI
i=1
while (( $i<= 4 ))
do
    cp -f ../mnt/player$i.cpp ./API/src
    mv ./API/src/player$i.cpp ./API/src/AI.cpp
    cmake ./CMakeLists.txt && make
    mv ./capi ../mnt/capi$i
    let "i++"
done