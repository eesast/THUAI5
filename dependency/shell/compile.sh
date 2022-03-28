#! /bin/bash 
# WORKDIR /usr/local/
cd ./CAPI
i=1
flag=1
while (( $i <= 4 ))
do
    cp -f ../mnt/player$i.cpp ./API/src
    mv ./API/src/player$i.cpp ./API/src/AI.cpp
    cmake ./CMakeLists.txt && make >build.log 2>&1
    mv ./build.log ../mnt/build_log$i && mv ./capi ../mnt/capi$i
    if [ $? -ne 0 ]; then
        flag=0
    fi
    let "i++"
done
if [ $flag -eq 1 ]; then
    curl http://localhost:28888/code/compileInfo -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer ${COMPILER_TOKEN}" -d '{"compile_status":"compiled"}'
else
    curl http://localhost:28888/code/compileInfo -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer ${COMPILER_TOKEN}" -d '{"compile_status":"failed"}'
fi