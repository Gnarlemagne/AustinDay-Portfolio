######################################################################
#
# CAS CS 320, Fall 2015
# Assignment 3 (skeleton code)
# interpret.py
#

exec(open('parse.py').read())

Node = dict
Leaf = str

def evalTerm(env, e):
	if type(e) == Node:
		for label in e:
			children = e[label]
			if label == 'Plus':
				f1 = children[0]
				f2 = children[1]
				v1 = evalTerm(env, f1)
				v2 = evalTerm(env, f2)
				return {'Number': [v1['Number'][0] + v2['Number'][0]]}
			elif label == 'Variable':
				x = children[0]
				if x in env:
					return env[x]
				else:
					print(x + " is unbound.")
					exit()

			elif label == 'Number':
				n = children[0]
				return {'Number': [n]}

def evalFormula(env, e):
	if type(e) == Node:
		for label in e:
			children = e[label]
			if label == 'And':
				v1 = children[0]
				v2 = children[1]
				f1 = evalFormula(env, v1)
				f2 = evalFormula(env, v2)
				if (f1 == 'True' and f2 == 'True'): 
					return 'True'
				else:
					return 'False'
			if label == 'Xor':
				v1 = children[0]
				v2 = children[1]
				f1 = evalFormula(env, v1)
				f2 = evalFormula(env, v2)
				if ((f1 == 'True' and f2 == 'False') or (f1 == 'False' and f2 == 'True')): 
					return 'True'
				else:
					return 'False'
			elif label == 'Not':
				f1 = children[0]
				v1 = evalFormula(env, f1)
				if v1 == 'True': return 'False'
				elif v1 == 'False': return 'True'
				else: return None
			elif label == 'Variable':
				x = children[0]
				if x in env:
					return env[x]
				else:
					print(x + " is unbound.")
					exit()
	elif type(e) == Leaf:
		if e == 'True':
			return 'True'
		if e == 'False':
			return 'False'		

def evalExpression(env, e): # Useful helper function.
	if type(e) == Node:
		for label in e:
			if label in ['Plus', 'Number', 'Variable']: 
				return evalTerm(env, e)

	return evalFormula(env, e)

def execProgram(env, s):
	if type(s) == Leaf:
		if s == 'End':
			return (env, [])
	elif type(s) == Node:
		for label in s:
			children = s[label]
			if label == 'Print':
				f = children[0]
				p = children[1]
				v = evalExpression(env, f)
				o = execProgram(env, p)
				return (env, [v] + o[1])
			if label == 'Assign':
				x = children[0]['Variable'][0]
				f = children[1]
				p = children[2]
				v = evalExpression(env, f)
				env[x] = v
				o = execProgram(env, p)
				return o
			if label == 'If':
				r = children[0]
				ex = evalExpression(env, r)
				n = children[2]
				o2 = execProgram(env, n)
				if ex == 'True':
					c = children[1]
					o1 = execProgram(env, c)
					return (env, o1[1] + o2[1])
				else:
					return (env, o2[1])
			if label == 'Until':
				ex = evalExpression(env, children[0])
				if ex == 'True':
					(env2, o1) = execProgram(env, children[2])
					return (env2, o1)
				else:
					(env2, o1) = execProgram(env, children[1])
					(env3, o2) = execProgram(env2, s)
					return (env3, o1 + o2)

			if label == 'Procedure':
				x = children[0]['Variable'][0]
				f = children[1]
				p = children[2]
				env[x] = f
				(env, o) = execProgram(env, p)
				return o
			if label == 'Call':
				x = children[0]['Variable'][0]
				p1 = env[x]
				p2 = children[1]
				(env2, o1) = execProgram(env, p1)
				(env3, o2) = execProgram(env2, p2)
				return (env3, o1 + o2)



def interpret(s):
	(env, o) = execProgram({}, tokenizeAndParse(s))
	return o

#eof
