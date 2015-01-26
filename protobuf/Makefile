LIBDIR=/usr/local/lib

CXXFLAGS=-g -Wall -O0 -pthread
LDFLAGS=-lprotobuf -lz -lpthread -Wl,-rpath -Wl,$(LIBDIR)

BINARIES=codec_test c_codec_test
TARGETS=$(BINARIES)

# comment out following line if you have boost installed
TARGETS=codec_test c_codec_test

all: $(TARGETS)

whole: $(BINARIES)

codec_test: reqresp.pb.h reqresp.pb.cc codec.h codec_test.cc

c_codec_test: reqresp.pb-c.h reqresp.pb-c.c c_codec_test.c

#$(BINARIES):
codec_test:
	g++ $(CXXFLAGS) $(filter %.cc,$^) -o $@ $(LDFLAGS)

c_codec_test:
	gcc -g -O0 --std=gnu99 c_codec_test.c reqresp.pb-c.c -lprotobuf-c -o c_codec_test

reqresp.pb.h reqresp.pb.cc: reqresp.proto
	protoc --cpp_out . $<
	protoc --python_out . $<

reqresp.pb-c.h reqresp.pb-c.c: reqresp.proto
	protocc --c_out=. reqresp.proto

test: codec_test
	./codec_test

clean:
	rm -f reqresp.pb.*
	rm -rf reqresp.pb-c.*
	rm -f reqresp_pb2.py*
	rm -f $(BINARIES)
