module Compile where

import Error
import AbstractSyntax
import TypeCheck
import Machine

convert :: Value -> Integer
convert b = -1 -- Complete for Problem 4, part (a).

converts :: Output -> Buffer
converts os = [] -- Complete for Problem 4, part (a).

type AddressEnvironment = [(Var, Address)]

addressVar :: Var -> AddressEnvironment -> Address
addressVar x' ((x,a):xas) = if x == x' then a else addressVar x xas

class Compilable a where
  compile :: AddressEnvironment -> a -> [Instruction]

instance Compilable Exp where -- 10 = output address, 11 = X, 12 = Y, 20 = stack bottom
  compile env (Variable v) = [SET 3 (addressVar v env), SET 4 10, COPY]
  compile env (Value True) = [SET 10 1]
  compile env (Value False) = [SET 10 0]
  compile env (And a1 a2) = (compile env a1) ++ [SET 3 10, SET 4 (20 + 1 + (size a1) + (size a2)), COPY] ++ (compile env a2) ++ [SET 3 10, SET 4 1, COPY, SET 3 (20 + 1 + (size a1) + (size a2)), SET 4 2, COPY, MUL, SET 3 0, SET 4 10, COPY]
  compile env (Or a1 a2) = (compile env a1) ++ [SET 3 10, SET 4 (20 + 1 + (size a1) + (size a2)), COPY] ++ (compile env a2) ++ [SET 3 10, SET 4 1, COPY, SET 3 (20 + 1 + (size a1) + (size a2)), SET 4 2, COPY, ADD, SET 3 0, SET 4 10, COPY]

instance Compilable Stmt where -- 9 = heap pointer, stores outputs starting at address -1, decreasing. I think I'm supposed to do something with address 5 but I'm lost and stressed and aahhhhhh
  compile env (Print e s) = [SET 1 9, SET 2 (-1), ADD, SET 3 0, SET 4 9, COPY] ++ (compile (env) (e)) ++ [SET 3 9, SET 4 4, COPY, SET 3 10, COPY] ++ (compile env (s))
  compile env (Assign v e s) = (compile env e) ++ [SET 3 10, SET 4 (addressVar v env), COPY] ++ compile env (s)
  compile env (End) = []
  compile env _     = [] 

compileSimulate  :: Stmt -> ErrorOr Buffer
compileSimulate s = if (check [] s == Result TyVoid) then Result (simulate (compile [(X, 11), (Y, 12)] s)) else promote (check [] s)

--eof