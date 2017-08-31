CCFLAGS +=-g -std=c++0x -I./include -Wall -I/usr/local/include/ -I/usr/include/postgresql/ -I./proto/
LDFLAGS +=-lpq -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/ -lprotobuf
PROTOBUF_URl=https://github.com/google/protobuf/releases/download/v3.4.0/protobuf-cpp-3.4.0.tar.gz
CURDIR=$(shell pwd)
main: protobuf
	$(CXX)  $(CCFLAGS) $(LDFLAGS) main.cpp proto/test.pb.cc -o main 

protobuf_dist: protobuf_lib
	mkdir protobuf_dist
	DIR=`pwd`
	cd protobuf_lib && ./configure --prefix $(CURDIR)/protobuf_dist
	$(MAKE) -C protobuf_lib install -j 8

protobuf_lib: protobuf.tar.gz
	mkdir tmp
	cd tmp && tar xvf ../protobuf.tar.gz
	cd tmp && mv * protobuf_lib
	mv tmp/protobuf_lib .
	rm -r tmp

protobuf.tar.gz:
	wget -qO protobuf.tar.gz $(PROTOBUF_URl)


clean:
	rm -rf main protobuf_lib protobuf.tar.gz

.PHONY: clean
