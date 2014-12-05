CXX_SOURCES =
CXXFLAGS = -g -Wall -std=c++11 -O3
LFLAGS =

LUA_CXXFLAGS = -Ilib/LuaJIT-2.0.3/src -Ilib/LuaState-master/include
LUA_LFLAGS = -Wl,-E -Llib/LuaJIT-2.0.3/src -Wl,-Bstatic -lluajit -Wl,-Bdynamic -lm -ldl

CXX_OBJECTS = $(CXX_SOURCES:.cc=.o)

src/%.o: src/%.cc
	$(CXX) $(LUA_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

all: dist/worker dist/boss dist/localworker

dist/boss: src/boss.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $(LFLAGS) $^

dist/worker: src/worker.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(LUA_CXXFLAGS) $(CXXFLAGS) $^ $(LUA_LFLAGS) $(LFLAGS)

dist/localworker: src/localworker.cc $(CXX_OBJECTS)
	$(CXX) -o $@ $(LUA_CXXFLAGS) $(CXXFLAGS) $^ $(LUA_LFLAGS) $(LFLAGS)

.PHONY: clean distclean

clean:
	-rm -rf **/*.o
	-rm -rf dist/*

distclean: clean
	-rm -rf lib/
