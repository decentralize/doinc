local bit = require('bit')

sandbox_env = {
  print = print,
  ipairs = ipairs,
  next = next,
  pairs = pairs,
  pcall = pcall,
  tonumber = tonumber,
  tostring = tostring,
  type = type,
  unpack = unpack,
  -- TODO: Decide whether corouting could be used to DOS a machine
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


  -- Libraries

  -- MsgPack
  mp = { pack = mp.pack, unpack = mp.unpack },

  -- dkjson
  json = { encode = json.encode, decode = json.decode, version = json.version, null = json.null },

  -- LuaJit Lua BitOp
  bit = { tobit = bit.tobit, tohex = bit.tohex, bnot = bit.bnot, band = bit.band,
      bor = bit.bor, bxor = bit.bxor, lshift = bit.lshift, rshift = bit.rshift,
      arshift = bit.arshift, rol = bit.rol, ror = bit.ror, bswap = bit.bswap},
}

function run_sandboxed(f)
  setfenv(f, sandbox_env)
  return pcall(f)
end

local blueprint = nil

function load_blueprint(code)
  local f = loadstring(code)
  e,f = run_sandboxed(f)
  if not e then
    local error_msg = f
    return false, error_msg
  end
  blueprint = f
  return true, ''
end

function perform_work(work)
  -- print('Starting work...')
  local e,res = pcall(blueprint, work)
  if not e then
    return false, res
  end
  return true, tostring(res)
end

-- function bc(func)
--   print('Function compiled: ', func)
-- end
-- jit.attach(bc, 'bc')
-- function trace(what, tr, func, pc, otr, oex)
--   print('Trace: ',  what)
-- end
-- jit.attach(trace, 'trace')
