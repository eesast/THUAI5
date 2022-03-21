@echo off

#start cmd /k ..\Test\bin\Debug\net5.0\Test.exe 0 0

start cmd /k ..\Test\bin\Debug\net5.0\Test.exe 0 1

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe -u --port=7777 --teamID=0 --characterID=0 --software=2 --hardware=3