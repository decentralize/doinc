function mandelbrot(cx,cy, max_iter, max)
  local x,y,xtemp,ytemp,squaresum,iter
  squaresum = 0
  x = 0
  y = 0
  iter = 0
  while (squaresum <= max) and (iter < max_iter)  do
    xtemp = x * x - y * y + cx
    ytemp = 2 * x * y + cy
    x = xtemp
    y = ytemp
    iter = iter + 1
    squaresum = x * x + y * y
  end
  local result = 0
  if (iter < max_iter) then
      result = iter
  end
  return result
end

function perform_work(w)
  local s = json.decode(w)
  local hscale = s.height / s.width
  local points = {}

  for col=0,s.width-1 do
    x = 3.0*col/s.width - 2.0
    y = 3.0*(s.row-1)/s.height*hscale - 1.5*hscale
    table.insert(points, mandelbrot(x, y, s.max_iter, s.max_diverge))
  end

  return json.encode({
    row = s.row,
    result = points
  })
end

return perform_work

