local WIDTH = 1920
local HEIGHT = 1080
local MAX_ITER = 10000
local MAX_DIVERGE = 4
local PER_WORK = 10

local finished_rows = {}
local ongoing_rows = {}
local done = 0

function initialize()
end

function is_done()
  return done >= HEIGHT
end

function get_work()
  local row = 0
  for r=1,HEIGHT,PER_WORK do
    if finished_rows[r] == nil and ongoing_rows[r] == nil then
      row = r
      break
    end
  end

  -- Finish up abandoned uncompleted
  if row == 0 then
    for r=1,HEIGHT,PER_WORK do
      if finished_rows[r] == nil then
        ongoing_rows[r] = nil
      end
    end

    return get_work()
  end

  ongoing_rows[row] = true
  local s = {
    height = HEIGHT,
    width = WIDTH,
    max_iter = MAX_ITER,
    max_diverge = MAX_DIVERGE,
    row = row,
    per_work = PER_WORK
  }
  return json.encode(s)
end

function accept_result(r)
  local s = json.decode(r)
  if s == nil or finished_rows[s.row] ~= nil then
    return
  end
  for r=0,PER_WORK-1 do
    finished_rows[r+s.row] = s.rows[r+1]
    ongoing_rows[r+s.row] = nil
  end
  done = done + PER_WORK

  io.write(string.format("\r%3.2d%% done (%d/%d)", done/HEIGHT*100, done, HEIGHT))
  io.flush()
end
