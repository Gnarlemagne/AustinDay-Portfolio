---------------------------------------------------------------------
--
-- CAS CS 320, Fall 2015
-- Assignment 5 (skeleton code)
-- Graph.hs
--

module Graph where

data Graph a =
    Choices a (Graph a, Graph a)
  | Outcome a
  deriving (Eq, Show)

class State a where
  outcome :: a -> Bool
  choices :: a -> (a, a)


mapTuple :: (a -> b) -> (a, a) -> (b, b)
mapTuple f (a1, a2) = (f(a1), f(a2))

state :: (Graph a) -> a
state (Choices a1 _) = a1
state (Outcome a1) = a1


-- If states can be compared, then graphs containing
-- those states can be compared by comparing the
-- states in the respective root nodes.
instance Ord a => Ord (Graph a) where
  g <  g' = state g < state g'
  g <=  g' = state g <= state g'


graph :: State a => a -> Graph a
graph (s) = if outcome s == True then Outcome (s) else Choices (s) (mapTuple graph (choices (s)))

depths :: Integer -> Graph a -> [Graph a]
depths 0 (g) = [g]
depths (n) (Outcome _) = []
depths (n) (Choices _ (g1, g2)) = (depths (n-1) (g1)) ++ (depths (n-1) (g2))

fold :: State a => (a -> b) -> (a -> (b, b) -> b) -> Graph a -> b
fold o c (Outcome g) = o (g)
fold o c (Choices s (g1, g2)) = c (s) ((fold o c g1),(fold o c g2))

outcomes :: State a => Graph a -> [Graph a]
outcomes (q) = fold (\o -> [Outcome o]) (\_ (g1, g2) -> (g1) ++ (g2)) (q)
--eof