module TypeCheck where

import Error
import AbstractSyntax

type TypeEnvironment = [(Var, Type)]

typeVar :: Var -> TypeEnvironment -> ErrorOr Type
typeVar x' ((x,t):xvs) = if x == x' then Result t else typeVar x' xvs
typeVar x'  _          = TypeError (show x' ++ " is not bound.")

class Typeable a where
  check :: TypeEnvironment -> a -> ErrorOr Type

instance Typeable Stmt where
  check env End = Result TyVoid
  check env (Print e s) = if (check env e == Result TyBool) then check env s else TypeError "Wrong type for Print (Expr)"
  check env (Assign v e s) = if (check env e == Result TyBool) then check (env ++ [(v,TyBool)]) (s) else TypeError "Wrong type for Var := (Expr)"

instance Typeable Exp where
  check env e = fold (\v -> typeVar (v) (env)) (\l -> Result TyBool) (\a1 a2 -> if a1 == Result TyBool && a2 == Result TyBool then Result TyBool else TypeError "Wrong Type for (Bool) and (Bool)") (\o1 o2 -> if o1 == Result TyBool && o2 == Result TyBool then Result TyBool else TypeError "Wrong Type for (Expr) or (Expr)") e

typeCheck :: Typeable a => a -> ErrorOr (a, Type)
typeCheck ast = liftErr (\t -> (ast, t)) (check [] ast) -- Pair result with its type.

--eof