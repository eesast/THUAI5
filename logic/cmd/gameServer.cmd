@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port 7777 --teamCount 2 --playerCount 1 --gameTimeInSecond 600 --mapResource=newmap.txt

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=1 --characterID=0 --software=2 --hardware=2

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 0 0
