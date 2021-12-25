
main :: IO ()
main = putStrLn $ show $ fib''' 16

fib :: Int -> Int
fib 0 = 0
fib 1 = 1
fib n = a + b
  where
    a = fib $ n - 1
    b = fib $ n - 2

fib' :: Int -> Int
fib' n = if n == 0 then 0
        else if n == 1 then 1
             else fib (n - 1) + fib (n - 2)

fib'' :: Int -> Int
fib'' n = case n of
  0 -> 0
  1 -> 1
  n -> fib'' (n - 1) + fib'' (n - 2)


fib''' :: Int -> Int
fib''' n
 | n == 0 = 0
 | n == 1 = 1
 | otherwise = fib''' (n - 1) + fib''' (n - 2)

x :: String -> Bool
x ('H':_) = True
x _       = False