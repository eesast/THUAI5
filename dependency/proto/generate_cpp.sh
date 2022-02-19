protoc --version
protoc --cpp_out=. Message2Clients.proto
protoc --cpp_out=. Message2Server.proto
protoc --cpp_out=. MessageType.proto
mv ./*.h ../../CAPI/proto
mv ./*.cc ../../CAPI/proto
