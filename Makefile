CXX_SOURCES = \
	src/protocol/packet.pb.cc

LUA_CXXFLAGS = -Ilib/LuaJIT-2.0.3/src -Ilib/LuaState-master/include
LUA_LFLAGS = -Llib/LuaJIT-2.0.3/src -lluajit -ldl

PROTOBUF_CXXFLAGS = -Ilib/protobuf-2.6.0/src
PROTOBUF_LFLAGS = -Llib/protobuf-2.6.0/src -lprotobuf

BOOST_LFLAGS = -lboost_system -pthread

CXX_OBJECTS = $(CXX_SOURCES:.cc=.o)
CXXFLAGS = -g -Wall -Werror -Wfatal-errors -std=c++11 $(PROTOBUF_CXXFLAGS) $(LUA_CXXFLAGS)
LFLAGS = -static $(BOOST_LFLAGS) $(PROTOBUF_LFLAGS) $(LUA_LFLAGS)

all: dist/client dist/nameserver dist/localworker

dist/nameserver: src/nameserver.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/client: src/client.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/localworker: src/localworker.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

src/protocol/packet.pb.cc: src/protocol/packet.proto
	cd src/protocol && ../../lib/protobuf-2.6.0/src/protoc --cpp_out=. packet.proto

**/%.o: **/%.cc
	$(CXX) $(LUA_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean distclean

clean:
	-rm -rf src/*.o
	-rm -rf src/**/*.o
	-rm -rf dist/*

distclean: clean
	-rm -rf lib/
