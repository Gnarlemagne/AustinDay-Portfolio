module Machine where

import Data.Char (toLower)
import Data.List (intercalate)

type Address = Integer
type Buffer = [Integer]
type Memory = [(Address, Integer)]
type MachineState = (Buffer, Memory)

data Instruction =
    SET Address Integer
  | COPY
  | ADD
  | MUL
  deriving (Eq, Show)

data Program = Program [Instruction]
instance Show Program where
  show (Program is) = map toLower (intercalate "\n" (map show is))

-- Get the integer at a memory address.
get :: Address -> MachineState -> Integer
get a' (buf, ((a,i):m)) = if a' == a then i else get a' (buf, m)
get _  _                = 0 -- Uninitialized memory addresses hold 0.

-- Set the memory address and update the buffer (if address 5 is set).
set :: Address -> Integer -> MachineState -> MachineState
set a' i' (buf, mem) =
  ( if a'==5 then buf++[i'] else buf
  , (a',i'):[(a,i) | (a,i) <- mem, a /= a']
  )

-- Update the machine state using one instruction.
step :: MachineState -> Instruction -> MachineState
step bufmem (SET a i) = set a i bufmem
step bufmem (COPY   ) = set (get 4 bufmem) (get (get 3 bufmem) bufmem) bufmem
step bufmem (MUL    ) = set 0 (get 1 bufmem * get 2 bufmem) bufmem
step bufmem (ADD    ) = set 0 (get 1 bufmem + get 2 bufmem) bufmem

simulate :: [Instruction] -> Buffer
simulate is = fst (foldl step ([], []) is)

--eof