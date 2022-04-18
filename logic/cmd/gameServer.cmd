@echo off


start cmd /k ..\Server\bin\Release\net5.0\Server.exe --port 7777 --teamCount 1 --playerCount 1 --gameTimeInSecond 6 --url http://localhost:28888/contest --token eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJyb29tX2lkIjoiN2Y5NTdmY2YtZDYwNC00NWMyLWFjODAtZDBlNTNjZDM3ZWY1IiwidGVhbV9pZHMiOlsiMGM1ZWJhNmUtYzg0ZC00NWFiLWFlNmQtN2I3N2ZkNmM3ZGE1IiwiODI5NjA0NGQtM2QxZS00ZTM4LTk1MjItMDBmZTllZDgxYjU1Il0sImlhdCI6MTY1MDE1NzQ2MCwiZXhwIjoxNjUwMTU5MjYwfQ.AtAV4FVgF3VyN4NrUq5bzvvZv_lKgQ5iWRgs-gAfw9w --fileName video 

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=0 --characterID=0 --software=4 --hardware=2


