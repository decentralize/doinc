sandbox_env = {
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
  os = { clock = os.clock, difftime = os.difftime, time = os.time }
}

function run_sandboxed(f)
  setfenv(f, sandbox_env)
  return pcall(f)
end

local bp_function = nil

function load_blueprint(bp_string)
  local f = loadstring(bp_string)
  if f == nil then
    return "Syntax error in blueprint"
  end
  e,bp_function = run_sandboxed(f)
  if e == false or bp_function == nil then
    return "Invalid blueprint - no work function returned"
  end
  return ''
end

function perform_work(work)
  -- print('Starting work...')
  return tostring(bp_function(work))
end

-- function bc(func)
--   print('Function compiled: ', func)
-- end
-- jit.attach(bc, 'bc')
-- function trace(what, tr, func, pc, otr, oex)
--   print('Trace: ',  what)
-- end
-- jit.attach(trace, 'trace')
