function fastfib(n)
  fibs={1,1}

  for i=3,n do
    fibs[i]=fibs[i-1]+fibs[i-2]
  end

  return fibs[n]
end

function wrapper(n)
  return fastfib(tonumber(n))
end

return wrapper
