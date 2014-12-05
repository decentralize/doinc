#include <iostream>
#include <fstream>
#include <string>

#include <LuaState.h>
#include <cassert>

using namespace std;

const char* const SANDBOX = R"(
  -- sample sandbox environment
  sandbox_env = {
    work = work,
    ipairs = ipairs,
    next = next,
    pairs = pairs,
    pcall = pcall,
    tonumber = tonumber,
    tostring = tostring,
    type = type,
    unpack = unpack,
    coroutine = { create = coroutine.create, resume = coroutine.resume,
        running = coroutine.running, status = coroutine.status,
        wrap = coroutine.wrap },
    string = { byte = string.byte, char = string.char, find = string.find,
        format = string.format, gmatch = string.gmatch, gsub = string.gsub,
        len = string.len, lower = string.lower, match = string.match,
        rep = string.rep, reverse = string.reverse, sub = string.sub,
        upper = string.upper },
    table = { insert = table.insert, maxn = table.maxn, remove = table.remove,
        sort = table.sort },
    math = { abs = math.abs, acos = math.acos, asin = math.asin,
        atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos,
        cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor,
        fmod = math.fmod, frexp = math.frexp, huge = math.huge,
        ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max,
        min = math.min, modf = math.modf, pi = math.pi, pow = math.pow,
        rad = math.rad, random = math.random, sin = math.sin, sinh = math.sinh,
        sqrt = math.sqrt, tan = math.tan, tanh = math.tanh },
    os = { clock = os.clock, difftime = os.difftime, time = os.time },
  }


  function run_sandboxed(f)
    setfenv(f, sandbox_env)
    return pcall(f)
  end

  -- io.write("# code:\n", blueprint, "\n")
  -- io.write("# work:\n", work, "\n")

  is_error = false
  error = nil
  result = "none"

  local f = loadstring(blueprint)
  if f == nil then
    is_error = true
    error = "Syntax error in blueprint"
    return
  end

  local ret, e = run_sandboxed(f)
  if ret then
    -- TODO: convert table to JSON
    result = tostring(e)
  else
    -- TODO: fuck shit up
    is_error = true
    error = tostring(e)
  end
)";

std::string runSandboxed(std::string code) {
  lua::State state;

  state.setString("blueprint", code);
  state.setInt("work", 1200);
  cout << "=== State ready, running ===" << endl;
  try {
    state.doString(SANDBOX);
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
