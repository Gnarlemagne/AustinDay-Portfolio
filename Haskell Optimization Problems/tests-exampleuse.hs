module HW5_Tests where

import Data.List (intercalate)

import Graph
import Algorithm
import BinPacking
import SuperString

-- Evaluating the below value runs all the tests.
allTests = putStrLn $ intercalate "\n\n" [
  intercalate "\n" $ checks "ordTests" ordTests,
  intercalate "\n" $ checks "stateDepthsOutcomesIntegerTests" stateDepthsOutcomesIntegerTests,
  intercalate "\n" $ checks "stateDepthsOutcomesTests" stateDepthsOutcomesTests,
  intercalate "\n" $ checks "greedyTests" greedyTests,
  intercalate "\n" $ checks "patientTests" patientTests,
  intercalate "\n" $ checks "optimalTests" optimalTests,
  intercalate "\n" $ checks "metaTests" metaTests
  ]

-- To get the failures for an individual test, query that
-- test using "checks", e.g.:
-- 
-- *> checks "ordTests" ordTests

checks :: (Show a, Eq a) => String -> [(Integer, a, a)] -> [String]
checks name tests = [check name n x y | (n, x, y) <- tests]

check :: (Show a, Eq a) => String -> Integer -> a -> a -> String
check name n x y =
  if x /= y then
    "In " ++ name ++ "failed #" ++ show n ++ ": " ++ show x ++ " =/= " ++ show y
  else
    "Passed " ++ name ++ " #" ++ show n ++ "."

ordTests = [
  (1, BinPack 0 0 [] <= BinPack 5 5 [], True),
  (2, BinPack 1 4 [5,6] < BinPack 5 10 [5,6], True),
  (3, Outcome "b" < Outcome "a", False),
  (4, Outcome 456 > Outcome 123, True),
  (5, Outcome (BinPack 4 8 []) < Outcome (BinPack 1 14 []), True),
  (6, Outcome (BinPack 4 8 [1,2,3]) < Outcome (BinPack 1 14 [1,2,3]), True),
  (7, Outcome (SuperStr "aaabc" ["a","x","yy"]) < Outcome (SuperStr "abc" ["a","x","yy"]), False),
  (8, Outcome (SuperStr "aaab" []) < Outcome (SuperStr "aaaab" []), True),
  (9, max (Outcome (BinPack 9 1 [5,7])) (Outcome (BinPack 13 12 [5,7])) == Outcome (BinPack 9 1 [5,7]), True)
  ]

-- Since the "Graph a" data structure is polymorphic, we can
-- let integers represent states in the state space graph.

instance State Integer where
  choices n = (n+1, n-1)
  outcome n = n == 0

stateDepthsOutcomesIntegerTests :: [(Integer, Integer, Integer)]
stateDepthsOutcomesIntegerTests = [
  (1, state $ Choices 1 (Outcome 0, Outcome 2), 1),
  (2, state $ maximum (depths 1 (Choices 1 (Outcome 0, Outcome 2))), 2),
  (3, state $ minimum (outcomes (Choices 5 (Choices 4 (Outcome 3, Outcome 5), Choices 6 (Outcome 5, Outcome 7)))), 3)
  ]

stateDepthsOutcomesTests = [
  (1, state $ graph (BinPack 0 0 [2,5,6,2,5,4,1,6]), BinPack 0 0 [2,5,6,2,5,4,1,6]),
  (2, state $ Outcome (BinPack 1 2 []), BinPack 1 2 []),
  (3, state $ minimum (depths 3 (graph (BinPack 0 0 [2,5,6,2,5,4,1,6]))), BinPack 7 6 [2,5,4,1,6]),
  (4, state $ maximum (outcomes (graph (BinPack 0 0 [1,4,3,1,7,3,4,4]))), BinPack 0 27 []),
  (5, (\(BinPack x y _) -> BinPack x y []) $ (state $ minimum (depths 5 (graph (BinPack 0 0 [1..])))), BinPack 7 8 [])
  ]

diff :: BinPacking -> Integer
diff (BinPack a b _) = abs (a - b)

len :: SuperString -> Integer
len (SuperStr s _) = toInteger $ length s

greedyTests = [
  (1, diff $ state (greedy (graph (BinPack 3 0 [2,1]))), 1),
  (2, diff $ state (greedy (greedy (graph (BinPack 3 0 [1,2])))), 0),
  (3, diff $ state (greedy (greedy (graph (BinPack 0 0 [5,2])))), 3),
  (4, diff $ state (greedy (graph (BinPack 0 0 [10,12,13]))), 10),
  (5, diff $ state (greedy $ greedy $ greedy $ greedy (graph (BinPack 0 0 [1..]))), 2)
  ]

patientTests = [
  (1, state (patient 0 (graph (SuperStr "x" ["a","b"]))), SuperStr "x" ["a","b"]),
  (2, state (patient 4 (graph (SuperStr "x" ["aa","ab","ba","bba","aa"]))), SuperStr "bbabaax" ["aa"]),
  (3, state (patient 5 (graph (SuperStr "aaaabb" ["aaa","bb","aabb","aaaa","aaaab"]))), SuperStr "aaaabb" [])
  ]

optimalTests = [
  (1, diff $ state (optimal (graph (BinPack 0 0 [2,1,3,2,1]))), 1),
  (2, diff $ state (optimal (graph (BinPack 9 0 [2,1,3,2,1]))), 0),
  (3, diff $ state (optimal (graph (BinPack 0 0 [2,5,6,2,5,4,1,6]))), 1),
  (4, diff $ state (optimal (graph (BinPack 0 0 [2,5,6,2,5,4,1,6,2,4,1,4,1,6]))), 1)
  ]

metaTests = [
  (1, len $ state (metaCompose (patient 2) (patient 2) (graph (SuperStr "aabbcc" ["aab","ccx","xa","xa","yxa"]))), 8),
  (2, diff $ state $ (patient 4) (graph (BinPack 0 0 [1,3,5,18,11])), 9),
  (3, len $ state (metaCompose greedy (patient 3) (graph (SuperStr "aaaabc" ["aaaa","bbaa","bca","ccccd","aaacc"]))), 14),
  (4, diff $ state (metaRepeat 4 (patient 2) (graph (BinPack 0 0 [1..8]))), 0),
  (5, len $ state (metaRepeat 2 (patient 2) (graph (SuperStr "" (map show [1..4])))), 4),
  (6, diff $ state (metaGreedy optimal (metaRepeat 5 greedy) (graph (BinPack 0 0 [2,1,3,2,1]))), 1),
  (7, diff $ state (metaGreedy (patient 2) (metaRepeat 2 greedy) (graph (BinPack 0 0 [6,1,7,2,6]))), 5),
  (8, len $ state (metaRepeat 500 (patient 2) (graph (SuperStr "" (map show [1..])))), 2774)
  ]

--eof