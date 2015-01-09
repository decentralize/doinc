#include "lua_runtime.h"

#include <string>
using std::string;

#include <exception>
using std::exception;

#include <LuaState.h>

#include "lua/sandbox.h"
#include "lua/msgpack.h"
#include "lua/dkjson.h"
#define SRC(x) string(x, x + sizeof x / sizeof x[0])

LuaRuntime::LuaRuntime(): state() {
  // Load hardcoded msgpack
  state.set("malloc", &malloc);
  state.set("realloc", &realloc);
  state.set("free", &free);
  state.doString(SRC(src_lua_msgpack_lua));
  state.doString(SRC(src_lua_dkjson_lua));
}


LuaSandbox::LuaSandbox(string code) : LuaRuntime() {
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

string LuaSandbox::PerformWork(string work) {
  bool success;
  string msg;
  lua::tie(success, msg) = state["perform_work"](work);
  if(success) {
    return msg;
  } else {
    throw(WorkException(msg));
  }
}

LuaSupplier::LuaSupplier(string code) : LuaRuntime() {
  // Load the supplier program
  try {
    state.doString(code);
  } catch(const exception &e) {
    throw(LuaException(e.what()));
  }

  // At this point initialize, is_done, get_work and accept_result
  // should all be defined on state
  string fns[3] = {"is_done", "get_work", "accept_result"};
  for(string fn : fns) {
    bool defined = state.doString(string("return ") + fn + " ~= nil");
    if(!defined) {
      throw(SupplierProgramException(fn + " is not defined."));
    }
  }

  try {
    bool defined = state.doString(string("return initialize ~= nil"));
    if(defined) {
      state["initialize"]();
    }
  } catch(const exception &e) {
    throw(LuaException(e.what()));
  }
}

bool LuaSupplier::IsDone() {
  try {
    return state["is_done"]();
  } catch(const exception &e) {
    throw(LuaException(e.what()));
  }
}

string LuaSupplier::GetWork() {
  try {
    return state["get_work"]();
  } catch(const exception &e) {
    throw(LuaException(e.what()));
  }
}

void LuaSupplier::AcceptResult(string result) {
  try {
    state["accept_result"](result);
  } catch(const exception &e) {
    throw(LuaException(e.what()));
  }
}
