CXX_SOURCES = \
	src/protocol/packet.pb.cc

LUA_CXXFLAGS = -Ilib/LuaJIT-2.0.3/src -Ilib/LuaState-master/include
LUA_LFLAGS = -Wl,-E -Llib/LuaJIT-2.0.3/src -Wl,-Bstatic -lluajit -Wl,-Bdynamic -lm -ldl

CXX_OBJECTS = $(CXX_SOURCES:.cc=.o)
CXXFLAGS = -Wfatal-errors -g -Wall -std=c++11 -O3 -Ilib/protobuf-2.6.0/src $(LUA_CXXFLAGS)
LFLAGS = -Llib/protobuf-2.6.0/src -lprotobuf $(LUA_LFLAGS)

src/protocol/packet.pb.cc: src/protocol/packet.proto
	cd src/protocol && ../../lib/protobuf-2.6.0/src/protoc --cpp_out=. packet.proto

**/%.o: **/%.cc
	$(CXX) $(LUA_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

all: dist/worker dist/supplier dist/localworker

dist/supplier: src/supplier.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/worker: src/worker.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

dist/localworker: src/localworker.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LFLAGS)

.PHONY: clean distclean

clean:
	-rm -rf src/*.o
	-rm -rf src/**/*.o
	-rm -rf dist/*

distclean: clean
	-rm -rf lib/
