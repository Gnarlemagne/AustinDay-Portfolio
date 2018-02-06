---------------------------------------------------------------------
--
-- CAS CS 320, Fall 2015
-- Assignment 5 (skeleton code)
-- BinPacking.hs
--

module BinPacking where

import Graph
import Algorithm

type Item = Integer
type Bin = Integer

data BinPacking =
    BinPack Bin Bin [Item]
    deriving (Eq, Show)


instance Ord BinPacking where
    (BinPack b1 b2 _) < (BinPack b3 b4 _) = (abs (b1-b2)) < (abs (b3-b4))
    (BinPack b1 b2 _) <= (BinPack b3 b4 _) = (abs (b1-b2)) <= (abs (b3-b4))

instance State BinPacking where
    outcome (BinPack _ _ []) = True
    outcome (BinPack _ _ _) = False
    choices (BinPack b1 b2 (x:xs)) = (BinPack (b1 + x) b2 (xs), BinPack b1 (b2 + x) (xs)) 


--eof