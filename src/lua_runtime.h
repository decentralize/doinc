#ifndef __LUA_SANDBOX_H_
#define __LUA_SANDBOX_H_

#include <string>
using std::string;

#include <LuaState.h>

#include <stdexcept>
using std::runtime_error;

class LuaException : public runtime_error {
public:
  LuaException(const char* s): LuaException(string(s)) {}
  LuaException(string s): runtime_error(s) {}
};
class SupplierProgramException : public runtime_error {
public:
  SupplierProgramException(const char* s): SupplierProgramException(string(s)) {}
  SupplierProgramException(string s): runtime_error(s) {}
};
class BlueprintException : public runtime_error {
public:
  BlueprintException(const char* s): BlueprintException(string(s)) {}
  BlueprintException(string s): runtime_error(s) {}
};
class WorkException : public runtime_error {
public:
  WorkException(const char* s): WorkException(string(s)) {}
  WorkException(string s): runtime_error(s) {}
};

class LuaRuntime {
protected:
  lua::State state;

public:
  LuaRuntime();
};

class LuaSandbox : public LuaRuntime {
public:
  LuaSandbox(string code);
  string PerformWork(string work);
};

class LuaSupplier : public LuaRuntime {
public:
  LuaSupplier(string code);
  bool IsDone();
  string GetWork();
  void AcceptResult(string result);
};


#endif
