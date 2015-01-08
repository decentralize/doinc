#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include <cassert>

#include <LuaState.h>

#include "lua/sandbox.h"
#include "lua/msgpack.h"

string runSandboxed(string code) {
  lua::State state;

  try {
    // Load msgpack and sandbox
    state.set("malloc", &malloc);
    state.set("realloc", &realloc);
    state.set("free", &free);
    state.doString(string(reinterpret_cast<const char*>(src_lua_msgpack_lua)));
    state.doString(string(reinterpret_cast<const char*>(src_lua_sandbox_lua)));
  } catch(const lua::RuntimeError& e) {
    cerr << "RuntimeError: " << e.what() << endl;
    return string();
  } catch(const lua::LoadError& e) {
    cerr << "LoadError: " << e.what() << endl;
    return string();
  }

  cout << "=== State ready, running ===" << endl;

  string lr = state["load_blueprint"](code);
  if(lr.length() > 0) {
    cerr << "Fail: " << lr << endl;
    return string();
  }

  int works[] = {1, 2, 3, 4, 5, 10, 50, 100, 150, 200, 250, 300};
  for (int &work : works) {
    string result = state["perform_work"](work);
    cout << "work(" << work << "): " << result << endl;
  }

  return string();
}

int main(int argc, char** argv) {
  for(int i = 1; i < argc; i++) {
    auto filename = argv[i];
    cout << "=== Running file: " << filename << " ===" << endl;
    ifstream ifs(filename);
    string content( (istreambuf_iterator<char>(ifs) ),
                       (istreambuf_iterator<char>()    ) );
    string ret = runSandboxed(content);
    cout << ret << endl << endl;
  }

  return 0;
}
