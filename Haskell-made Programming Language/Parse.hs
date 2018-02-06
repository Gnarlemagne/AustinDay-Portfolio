module Parse where

import Error
import AbstractSyntax

type Token = String

tokenize :: String -> [Token]
tokenize s =
  let splits = [0] ++ concat [[i,i+1] | i <- [0..length s-1], s!!i `elem` " "] ++ [length s]
      all = [ [s!!i | i <- [splits!!k..(splits!!(k+1)-1)]] | k <- [0..length splits-2] ]
  in [token | token <- all, token /= " " && token /= ""]

parser :: (Exp -> Exp) -> ([Token] -> ErrorOr (Exp, [Token]))
parser f ts =
  case parse ts of
    (Result (e, ts)) -> Result (f e, ts)
    (err           ) -> err

parsers :: (Exp -> Stmt -> Stmt) -> ([Token] -> ErrorOr (Stmt, [Token]))
parsers f ts =
  case parse ts of
    (Result (e, ts)) ->
      case parse ts of
        (Result (s, ts)) -> Result (f e s, ts)
        (err           )-> err
    (err           ) -> promote err

class Parseable a where
  parse :: [Token] -> ErrorOr (a, [Token])

instance Parseable Exp where
  parse ("x":ts) = Result (Variable X, ts)
  parse ("y":ts) = Result (Variable Y, ts)
  parse ("True":ts) = Result (Value True, ts)
  parse ("False":ts) = Result (Value False, ts)
  parse ("x":("and":ts)) = parser (And (Variable X)) ts
  parse ("y":("and":ts)) = parser (And (Variable Y)) ts
  parse ("True":("and":ts)) = parser (And (Value True)) ts
  parse ("False":("and":ts)) = parser (And (Value False)) ts
  parse ("x":("or":ts)) = parser (Or (Variable X)) ts
  parse ("y":("or":ts)) = parser (Or (Variable Y)) ts
  parse ("True":("or":ts)) = parser (Or (Value True)) ts
  parse ("False":("or":ts)) = parser (Or (Value False)) ts

  parse (t:_   ) = ParseError ("Token '" ++ t ++ "' a not valid way to begin an expression.")
  parse _        = ParseError ("Failed to parse expression.")

instance Parseable Stmt where
  parse ("x":("=":ts)) = parsers (Assign X) ts
  parse ("y":("=":ts)) = parsers (Assign Y) ts
  parse ("print":ts) = parsers (Print) ts
  
  parse (t:_)          = ParseError ("Token '" ++ t ++ "' a not valid way to begin a statement.")
  parse []             = Result (End, [])

tokenizeParse :: String -> ErrorOr Stmt
tokenizeParse s = liftErr (fst) (parse (tokenize s))

-- Examples of concrete syntax strings being parsed.
example1 = parse (tokenize "x = x   x = x   x = x") :: ErrorOr (Stmt, [Token])
example2 = parse (tokenize "x = True y = False print x and y") :: ErrorOr (Stmt, [Token])

--eof