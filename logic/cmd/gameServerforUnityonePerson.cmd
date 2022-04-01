@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=1 --playerCount=1  --gameTimeInSecond=100 --mapResource=testmap1.txt 

