module AbstractSyntax where

type Value = Bool
type Output = [Value]

data Var = X | Y deriving Eq

instance Show Var where
  show X = "x"
  show Y = "y"

data Stmt =
    Print Exp Stmt
  | Assign Var Exp Stmt
  | End
  deriving (Eq, Show)

data Exp =
    Variable Var
  | Value Bool
  | And Exp Exp
  | Or Exp Exp
  deriving (Eq, Show)

data Type =
    TyBool
  | TyVoid
  deriving (Eq, Show)

fold :: (Var -> b) -> (Bool -> b) -> (b -> b -> b) -> (b -> b -> b) -> Exp -> b
fold v l a o (Variable r) = v (r)
fold v l a o (Value r) = l (r)
fold v l a o (And r1 r2) = a (fold v l a o r1) (fold v l a o r2)
fold v l a o (Or r1 r2) = o (fold v l a o r1) (fold v l a o r2)

size :: Exp -> Integer
size e = fold (\v -> 1) (\l -> 1) (\a1 a2 -> 1 + a1 + a2) (\o1 o2 -> 1 + o1 + o2) e

foldStmt :: (Exp -> b -> b) -> (Var -> Exp -> b -> b) -> b -> Stmt -> b
foldStmt p a n (Print e s) = p (e) (foldStmt p a n s)
foldStmt p a n (Assign v e s) = a (v) (e) (foldStmt p a n s)
foldStmt p a n (End) = n


--eof