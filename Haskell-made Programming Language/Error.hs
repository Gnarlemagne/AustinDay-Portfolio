module Error where

data ErrorOr a =
    Result a
  | ParseError String
  | TypeError String
  deriving Show

promote :: ErrorOr a -> ErrorOr b
promote (ParseError s     ) = ParseError s
promote (TypeError s      ) = TypeError s

instance Eq a => Eq (ErrorOr a) where
  Result r1 == Result r2 = r1 == r2
  _ == _ = False

liftErr :: (a -> b) -> (ErrorOr a -> ErrorOr b)
liftErr (f) (Result r) = Result (f(r))
liftErr _ (ParseError e) = ParseError e
liftErr _ (TypeError e) = TypeError e

joinErr :: ErrorOr (ErrorOr a) -> ErrorOr a
joinErr (Result (Result r)) = Result r
joinErr (Result (ParseError e)) = ParseError e
joinErr (Result (TypeError e)) = TypeError e
joinErr (ParseError e) = ParseError e
joinErr (TypeError e) = TypeError e

--eof