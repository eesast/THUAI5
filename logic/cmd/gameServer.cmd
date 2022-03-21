@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=1  --gameTimeInSecond=60 --mapResource=testmap1.txt 

#start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=5000 --characterID=5000 --software=3 --hardware=3