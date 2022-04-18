@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=4  --gameTimeInSecond=600 --mapResource=testmap1.txt

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=0 --characterID=0 --software=3 --hardware=2