-- Step 1: Create storage for finished parts of the bitmap - files?
-- Step 2: Divide the problem into parts - ranges?
-- Step 3: Define functions for giving out work and accepting results

-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Mike Pall
-- with optimizations by Markus J. Q. (MarkusQ) Roberts

local width = 1000
local height = 1000

local max_iter = 10000
local max_diverge = 4

local points = {}

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

for row=0,height-1 do
    for col=0,width-1 do
        x = 2.0*col/width - 1.0
        y = 2.0*row/height - 1.0
        points[row*width+col] = mandelbrot(x, y, max_iter, max_diverge)
    end
end

print("Done!");
