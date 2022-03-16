@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=1  --gameTimeInSecond=300 --mapResource=testmap1.txt

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=3330 --characterID=3330 --software=2 --hardware=2