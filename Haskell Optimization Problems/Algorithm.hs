---------------------------------------------------------------------
--
-- CAS CS 320, Fall 2015
-- Assignment 5 (skeleton code)
-- Algorithm.hs
--

module Algorithm where

import Graph

type Algorithm a = Graph a -> Graph a


-- Complete Problem #4, parts (a-f).
greedy :: Ord a => Algorithm a
greedy (Choices _ (g, g')) = if g < g' then g else g'

patient :: Ord a => Integer -> Algorithm a
patient 0 g = g
patient n g = minimum (depths n g)

optimal :: (State a, Ord a) => Algorithm a
optimal g = minimum (outcomes g)

metaCompose :: Algorithm a -> Algorithm a -> Algorithm a
metaCompose a a' g = a (a' (g))

metaRepeat :: Integer -> Algorithm a -> Algorithm a
metaRepeat 0 _ g = g
metaRepeat n a g = metaRepeat (n-1) a (a(g))

metaGreedy :: Ord a => Algorithm a -> Algorithm a -> Algorithm a
metaGreedy a a' g = if a(g) < a'(g) then a(g) else a'(g)

-- Problem #4, part (g).
impatient :: Ord a => Integer -> Algorithm a
impatient n g = (metaRepeat n greedy) g

--impatient uses greedy to find a solution at depth n, taking far less time but getting a lower quality answer
--patient lays out all of the nodes at depth n to find the objectively best solution at that depth, but takes far longer
--in short, impatient is faster than patient, but gives worse results.


---------------------------------------------------------------------
-- Problem #6 (extra extra credit).

-- An embedded language for algorithms.
data Alg =
    Greedy
  | Patient Integer
  | Impatient Integer
  | Optimal
  | MetaCompose Alg Alg
  | MetaRepeat Integer Alg
  | MetaGreedy Alg Alg
  deriving (Eq, Show)

interpret :: (State a, Ord a) => Alg -> Algorithm a
interpret _ = \g -> g -- Replace for Problem #6, part (a).

data Time =
    N Integer 
  | Infinite
  deriving (Eq, Show)

-- instance Num Time where
--   ... Complete for Problem #6, part (b).

performance :: Alg -> Time
performance _ = N 0 -- Replace for Problem #6, part (c).

--eof