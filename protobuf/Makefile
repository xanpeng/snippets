LIBDIR=/usr/local/lib

CXXFLAGS=-g -Wall -O0 -pthread
LDFLAGS=-lprotobuf -lz -lpthread -Wl,-rpath -Wl,$(LIBDIR)

BINARIES=codec_test
TARGETS=$(BINARIES)

# comment out following line if you have boost installed
TARGETS=codec_test

all: $(TARGETS)

whole: $(BINARIES)

codec_test: reqresp.pb.h reqresp.pb.cc codec.h codec_test.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(filter %.cc,$^) -o $@ $(LDFLAGS)

reqresp.pb.h reqresp.pb.cc: reqresp.proto
	protoc --cpp_out . $<
	protoc --python_out . $<

test: codec_test
	./codec_test

clean:
	rm -f reqresp.pb.*
	rm -f reqresp_pb2.py*
	rm -f $(BINARIES)
