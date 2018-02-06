module Interpret where

import Error
import AbstractSyntax
import TypeCheck

type ValueEnvironment = [(Var, Value)]

valueVar :: Var -> ValueEnvironment -> Value
valueVar x' ((x,v):xvs) = if x == x' then v else valueVar x' xvs

evaluate :: ValueEnvironment -> Exp -> Value
evaluate env e = fold (\v -> valueVar (v) (env)) (\l -> l) (\a1 a2 -> if a1 == True && a2 == True then True else False) (\o1 o2 -> if o1 == True || o2 == True then True else False) e

execute :: ValueEnvironment -> Stmt -> (ValueEnvironment, Output)
execute env End = (env, []) -- Complete for Problem 2, part (b).
execute env (Print e s) = (fst(execute (env) (s)), ((evaluate env e) : snd(execute (env) (s))))
execute env (Assign v e s) = execute ((v, (evaluate env e)) : env) s

interpret :: Stmt -> ErrorOr Output
interpret s = if (check [] s == Result TyVoid) then Result (snd(execute [] s)) else promote (check [] s)

--eof