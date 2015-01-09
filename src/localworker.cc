#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;
using std::istreambuf_iterator;

#include <string>
using std::string;

#include "lua_runtime.h"

int main(int argc, char** argv) {
  try {
    ifstream supplier_ifs("server.lua");
    if(!supplier_ifs.good()) {
      cerr << "server.lua does not exist" << endl;
      return 1;
    }
    string supplier_code((istreambuf_iterator<char>(supplier_ifs)), (istreambuf_iterator<char>()));
    LuaSupplier supplier(supplier_code);
    cout << "# server.lua loaded" << endl;

    ifstream worker_ifs("client.lua");
    if(!worker_ifs.good()) {
      cerr << "client.lua does not exist" << endl;
      return 1;
    }
    string worker_code((istreambuf_iterator<char>(worker_ifs)), (istreambuf_iterator<char>()));
    LuaSandbox worker_sandbox(worker_code);
    cout << "# client.lua loaded" << endl;

    cout << "# Starting work cycle (1 worker)" << endl;
    while(!supplier.IsDone()) {
      string work = supplier.GetWork();
      string result = worker_sandbox.PerformWork(work);
      supplier.AcceptResult(result);
    }
  } catch(const SupplierProgramException &e) {
    cerr << "Invalid supplier program: " << e.what() << endl;
    return 1;
  } catch(const BlueprintException &e) {
    cerr << "Invalid blueprint: " << e.what() << endl;
    return 1;
  } catch(const WorkException &e) {
    cerr << "Error performing client work: " << e.what() << endl;
    return 1;
  } catch(const LuaException &e) {
    cerr << "Lua error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
