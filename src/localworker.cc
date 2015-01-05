#include <iostream>
#include <fstream>
#include <string>

#include <LuaState.h>
#include <cassert>

using namespace std;

#include "lua/sandbox.h"
#include "lua/msgpack.h"

std::string runSandboxed(std::string code) {
  lua::State state;

  state.setString("blueprint", code);
  state.setInt("work", 1200);

  cout << "=== State ready, running ===" << endl;
  try {
    state.set("malloc", &std::malloc);
    state.set("realloc", &std::realloc);
    state.set("free", &std::free);
    state.doString(std::string(reinterpret_cast<const char*>(src_lua_msgpack_lua)));
    state.doString(std::string(reinterpret_cast<const char*>(src_lua_sandbox_lua)));
  } catch(const lua::RuntimeError& e) {
    cerr << "Error: " << e.what() << endl;
    return std::string();
  }
  if(state["is_error"]) {
    std::string e = state["error"];
    cerr << "Error: " << e << endl;
    return std::string();
  }
  cout << "=== Ran successfully ===" << endl;
  return state["result"];
}

int main(int argc, char** argv) {
  for(int i = 1; i < argc; i++) {
    auto filename = argv[i];
    cout << "=== Running file: " << filename << " ===" << endl;
    std::ifstream ifs(filename);
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()    ) );
    string ret = runSandboxed(content);
    cout << ret << endl << endl;
  }

  return 0;
}
