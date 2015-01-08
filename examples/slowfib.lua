function slowfib(n)
  if (n <= 2) then
    return 1
  end

  return slowfib(n-1) + slowfib(n-2)
end

return slowfib
