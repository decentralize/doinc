local width = 1920
local height = 1080
local max_iter = 10000
local max_diverge = 4

local finished_rows = {}
local ongoing_rows = {}
local done = 0

function initialize()
  a = {}
  a[3] = true
  a[8] = true
  a[55] = true
  print(#a)
end

function is_done()
  return done == height
end

function get_work()
  local row = 0
  for r=1,height do
    if finished_rows[r] == nil and ongoing_rows[r] == nil then
      row = r
      break
    end
  end
  ongoing_rows[row] = true
  local s = {
    height = height,
    width = width,
    max_iter = max_iter,
    max_diverge = max_diverge,
    row = row
  }
  return json.encode(s)
end

function accept_result(r)
  local s = json.decode(r)
  finished_rows[s.row] = s
  ongoing_rows[s.row] = nil
  done = done + 1

  io.write(string.format("\r%3.2d%% done (%d/%d)", done/height*100, done, height))
  io.flush()
end
