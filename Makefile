CXX_SOURCES = \
	src/utils.cc \
	src/crypto.cc \
	src/lua_runtime.cc \
	src/protocol/packet.pb.cc
OTHER_SOURCES = \
  src/lua/sandbox.h \
  src/lua/msgpack.h \
  src/lua/dkjson.h

LUA_CXXFLAGS = -Ilib/LuaJIT-2.0.3/src -Ilib/LuaState-master/include
LUA_LFLAGS = -Llib/LuaJIT-2.0.3/src -lluajit -ldl

PROTOBUF_CXXFLAGS = -Ilib/protobuf-2.6.0/src
PROTOBUF_LFLAGS = -Llib/protobuf-2.6.0/src/.libs -lprotobuf

CRYPTOPP_CXXFLAGS = -Ilib/cryptopp562
CRYPTOPP_LFLAGS = -Llib/cryptopp562 -lcryptopp

BOOST_LFLAGS = -lboost_system -pthread

CXX_OBJECTS = $(CXX_SOURCES:.cc=.o)
CXXFLAGS = -g -Os -std=c++11 \
	-Wall -Werror -Wfatal-errors \
	-Wno-sign-compare -Wno-unused-variable -Wno-unused-function \
  $(PROTOBUF_CXXFLAGS) $(LUA_CXXFLAGS) $(CRYPTOPP_CXXFLAGS)
LFLAGS = -static $(BOOST_LFLAGS) $(PROTOBUF_LFLAGS) $(LUA_LFLAGS) $(CRYPTOPP_LFLAGS)

all: dist/supplier dist/nameserver dist/localworker dist/worker

dist/nameserver: src/nameserver.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/supplier: src/supplier.cc $(CXX_OBJECTS) $(OTHER_SOURCES)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/localworker: src/localworker.cc $(CXX_OBJECTS) $(OTHER_SOURCES)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/worker: src/worker.cc $(CXX_OBJECTS) $(OTHER_SOURCES)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

src/protocol/packet.pb.cc: src/protocol/packet.proto
	cd src/protocol && ../../lib/protobuf-2.6.0/src/protoc --cpp_out=. packet.proto

src/lua/%.h: src/lua/%.lua
	xxd -i $< $@

src/lua_runtime.o: src/lua_runtime.cc $(OTHER_SOURCES)
	$(CXX) $(LUA_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

**/%.o: **/%.cc Makefile
	$(CXX) $(LUA_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean distclean

clean:
	-rm -rf src/*.o
	-rm -rf src/**/*.o
	-rm -rf dist/*

distclean: clean
	-rm -rf lib/
