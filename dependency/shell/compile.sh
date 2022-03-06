#! /bin/bash
mv -f /usr/local/mnt/play$1.cpp ./API/src/AI.cpp
cmake CMakeLists.txt && make