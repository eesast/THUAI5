#! /bin/bash 
# WORKDIR /usr/local/
cd ./CAPI
i=1
flag=1
while (( $i <= 4 ))
do
    cp -f ../mnt/player$i.cpp ./API/src
    mv ./API/src/player$i.cpp ./API/src/AI.cpp
    cmake ./CMakeLists.txt && make >compile_log$i.txt 2>&1
    mv ./capi ../mnt/capi$i
    if [ $? -ne 0 ]; then
        flag=0
    fi
    mv ./compile_log$i.txt ../mnt/compile_log$i.txt
    let "i++"
done

if [ $flag -eq 1 ]; then
    curl $URL -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"compile_status":"compiled"}' > ../mnt/curl_log.txt
else
    curl $URL -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"compile_status":"failed"}' > ../mnt/curl_log.txt
fi