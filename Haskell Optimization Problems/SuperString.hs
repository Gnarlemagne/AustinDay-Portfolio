---------------------------------------------------------------------
--
-- CAS CS 320, Fall 2015
-- Assignment 5 (skeleton code)
-- SuperString.hs
--

module SuperString where

import Data.List (isPrefixOf)
import Graph
import Algorithm

data SuperString =
    SuperStr String [String]
  deriving (Eq, Show)

-- To merge two strings, take the longest suffix of the first string
-- that overlaps with the second, and replace it with the second string.
merge :: String -> String -> String
merge (x:xs) ys  = if isPrefixOf (x:xs) ys then ys else x:(merge xs ys)
merge []     ys  = ys


instance Ord SuperString where
    SuperStr a _ < SuperStr b _ = length a < length b
    SuperStr a _ <= SuperStr b _ = length a <= length b

instance State SuperString where
    outcome (SuperStr _ []) = True
    outcome (SuperStr _ _) = False
    choices (SuperStr s (x:xs)) = (SuperStr (merge s x) (xs), SuperStr (merge x s) (xs))


--eof