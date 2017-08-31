proto:
	protoc -I=`pwd` --go_out=`pwd` --cpp_out=`pwd`/cpp/proto `pwd`/test.proto
