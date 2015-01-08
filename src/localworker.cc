#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;
using std::istreambuf_iterator;

#include <string>
using std::string;

#include <stdexcept>
using std::runtime_error;

#include <LuaState.h>

#include "lua/sandbox.h"
#include "lua/msgpack.h"
#define SRC(x) string(x, x + sizeof x / sizeof x[0])

class LuaSandbox {
private:
  lua::State state;

public:
  class BlueprintException : public runtime_error {
  public:
    BlueprintException(string s): runtime_error(s) {}
  };
  class WorkException : public runtime_error {
  public:
    WorkException(const char* s): WorkException(string(s)) {}
    WorkException(string s): runtime_error(s) {}
  };

  LuaSandbox(string code):
      state() {

    // Load hardcoded msgpack
    state.set("malloc", &malloc);
    state.set("realloc", &realloc);
    state.set("free", &free);
    state.doString(SRC(src_lua_msgpack_lua));

    // Load sandbox: defines load_blueprint and perform_work
    state.doString(SRC(src_lua_sandbox_lua));

    // Load blueprint code into sandbox - gets JIT compiled immediately
    bool success;
    string load_error;
    lua::tie(success, load_error) = state["load_blueprint"](code);
    if(!success) {
      throw(BlueprintException(load_error));
    }
  }

  string PerformWork(string work) {
    bool success;
    string msg;
    lua::tie(success, msg) = state["perform_work"](work);
    if(success) {
      return msg;
    } else {
      throw(WorkException(msg));
    }
  }
};

int main(int argc, char** argv) {
  for(int i = 1; i < argc; i++) {
    auto filename = argv[i];
    ifstream ifs(filename);
    string code((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

    try {
      LuaSandbox sb(code);
      cout << "=== Running file: " << filename << " ===" << endl;
      string ret = sb.PerformWork("10");
      cout << "Success: " << ret << endl << endl;
    } catch(const LuaSandbox::BlueprintException &e) {
      cerr << "Invalid blueprint: " << e.what() << endl;
    } catch(const LuaSandbox::WorkException &e) {
      cerr << "Work failed: " << e.what() << endl;
    }
  }

  return 0;
}
