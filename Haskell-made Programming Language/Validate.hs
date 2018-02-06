module Exhaustive where

import Error
import AbstractSyntax
import Parse
import TypeCheck
import Interpret
import Compile
import Machine

type Height = Integer
type Quantity = Integer

class Exhaustive a where
  exhaustive :: Integer -> [a]

instance Exhaustive Stmt where
  exhaustive 0 = [] -- Complete for Problem 4, part (b).

instance Exhaustive Exp where
  exhaustive 0 = []  -- Complete for Problem 4, part (b).

take' :: Integer -> [a] -> [a]
take' n (x:xs) = x:(take' (n-1) xs)
take' _ _      = []

validate :: Height -> Quantity -> Bool
validate n k = False -- Complete for Problem 4, part (c).

complete :: String -> ErrorOr Buffer
complete _ = TypeError "Complete for Problem 4, part (d)."

--eof