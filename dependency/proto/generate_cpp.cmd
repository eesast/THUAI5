protoc --cpp_out=. Message2Clients.proto
protoc --cpp_out=. Message2Server.proto
protoc --cpp_out=. MessageType.proto
copy .\*.h ..\..\CAPI\CAPI\src\proto\*.h
copy .\*.cc ..\..\CAPI\CAPI\src\proto\*.cc
del *.h
del *.cc
