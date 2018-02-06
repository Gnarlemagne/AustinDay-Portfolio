import re

############################################################
# Load the files. Change the path if necessary.

exec(open('parse.py').read())
exec(open('interpret.py').read())
exec(open('machine.py').read())
exec(open('compile.py').read())

interpretCheck = open('interpret.py').read()
compileCheck = open('compile.py').read()
if interpretCheck.find('import parse') != -1 or\
   interpretCheck.find('from parse import') != -1 or\
   interpretCheck.find("exec(open('parse.py').read())") == -1:
    print('''You did not load the parse.py module correctly in interpret.py. You must use "exec(open('parse.py').read())". Exiting.''')
    exit()
if compileCheck.find('import parse') != -1 or\
   compileCheck.find('from parse import') != -1 or\
   compileCheck.find("exec(open('parse.py').read())") == -1:
    print('''You did not load the parse.py module correctly in compile.py. You must use "exec(open('parse.py').read())". Exiting.''')
    exit()
if compileCheck.find('import machine') != -1 or\
   compileCheck.find('from machine import') != -1 or\
   compileCheck.find("exec(open('machine.py').read())") == -1:
    print('''You did not load the machine.py module correctly in compile.py. You must use "exec(open('machine.py').read())". Exiting.''')
    exit()

def check(name, function, inputs_result_pairs):
    def str_(s): return '"'+str(s)+'"' if type(s) == str else str(s)
    if type(name) == tuple:
        prefix = name[0]
        suffix = name[1]
    if type(name) == str:
        prefix = name + '('
        suffix = ')'
    
    passed = 0
    for (inputs, result) in inputs_result_pairs:
        try:
            if len(inputs) == 1:
                output = function(inputs[0])
            if len(inputs) == 2:
                output = function(inputs[0], inputs[1])
            if len(inputs) == 3:
                output = function(inputs[0], inputs[1], inputs[2])
        except:
            output = None

        if output == result:
            passed = passed + 1
        else:
            print("\n  Failed on:\n    "+prefix+', '.join([str_(i) for i in inputs])+suffix+"\n\n"+"  Should be:\n    "+str(result)+"\n\n"+"  Returned:\n    "+str(output)+"\n")
    print("Passed " + str(passed) + " of " + str(len(inputs_result_pairs)) + " tests.")
    print("")

############################################################
# The tests.

print("Problem #1, part (a), evalTerm()...")
try: evalTerm
except: print("The evalTerm() function is not defined.")
else: check('evalTerm', evalTerm, [\
        (({}, {'Number': [123]}), {'Number': [123]}),\
        (({}, {'Plus': [{'Number': [0]}, {'Number': [1]}]}), {'Number': [1]}),\
        (({'x':{'Number':[123]}, 'y':{'Number':[123]}}, {'Plus': [{'Variable': ['x']}, {'Variable': ['y']}]}), {'Number': [246]}),\
        ])

print("Problem #1, part (c), execProgram()...")
try: execProgram
except: print("The execProgram() function is not defined.")
else: check('execProgram', (lambda env, stmt: execProgram(env, stmt)[1]), [\
        (({'y':'False'}, {'Print': [{'Not':[{'Variable':['y']}]}, 'End']}), ['True']),\
        (({}, {'If': ['True', {'Print': [{'Number': [4]}, 'End']}, 'End']}), [{'Number':[4]}]),\
        (({}, {'Assign': [{'Variable': ['x']}, 'False', {'Until': [{'Not': [{'Variable': ['x']}]}, {'Print': ['False', 'End']}, {'Print': ['True', 'End']}]}]}), ['True']),\
        (({}, {'If': ['True', {'Print': ['True', 'End']}, {'Print': ['False', 'End']}]}), ['True', 'False']),\
        (({}, {'Procedure': [{'Variable': ['example']}, {'Print': [{'Number': [4]}, 'End']}, {'Call': [{'Variable': ['example']}, 'End']}]}), [{'Number':[4]}]),\
        (({}, {'Assign': [{'Variable': ['x']}, {'Number': [123]}, {'Procedure': [{'Variable': ['example']}, {'Print': [{'Variable': ['x']}, 'End']}, {'Call': [{'Variable': ['example']}, {'Call': [{'Variable': ['example']}, 'End']}]}]}]}), [{'Number':[123]}, {'Number':[123]}]),\
        (({}, {'Procedure': [{'Variable': ['g']}, {'Print': [{'Number': [2]}, 'End']}, {'Procedure': [{'Variable': ['f']}, {'Call': [{'Variable': ['g']}, {'Print': [{'Number': [1]}, {'Call': [{'Variable': ['g']}, 'End']}]}]}, {'Call': [{'Variable': ['f']}, 'End']}]}]}), [{'Number':[2]}, {'Number':[1]}, {'Number':[2]}]),\
        ])

print("Problem #1, interpret()...")
try: interpret
except: print("The interpret() function is not defined.")
else: check('interpret', interpret, [\
    (("print 123;",), [{'Number':[123]}]),\
    (("print false; print true; print 4;",), ['False', 'True', {'Number':[4]}]),\
    (("x := 10; print x;",), [{'Number':[10]}]),\
    (("x := 10; print x + x;",), [{'Number':[20]}]),\
    (("x := 1; y := 2; z := 3; print x + y + z;",), [{'Number':[6]}]),\
    (("print true and false;",), ['False']),\
    (("x := true xor false; print x;",), ['True']),\
    (("x := true xor false; print x and not(x);",), ['False']),\
    (("x := true; y := false; z := true; print (not(x) xor y) and z;",), ['False']),\
    (("if true {print 4;}",), [{'Number':[4]}]),\
    (("if true {print true;} print false;",), ['True','False']),\
    (("x := false; until not(x) {print false;} print true;",), ['True']),\
    (("if true {print true;} print false;",), ['True','False']),\
    (("procedure f { } print 4;",), [{'Number':[4]}]),\
    (("procedure example {print 4;} call example;",), [{'Number':[4]}]),\
    (("x := 123; procedure example {print x;} call example; call example;",), [{'Number':[123]},{'Number':[123]}]),\
    (("procedure g {print 2;} procedure f {call g; print 1; call g;} call f;",), [{'Number':[2]},{'Number':[1]},{'Number':[2]}]),\
    (("procedure g {print 2;} if true and true { call g; }",), [{'Number':[2]}]),\
    (("procedure g {print 2;} procedure f {if true and true { call g; }} call g; call f;",), [{'Number':[2]},{'Number':[2]}]),\
    (("procedure h {print 3;} procedure g {print 2; call h; call h;} procedure f {call g; print 1; call g;} call f;",), [{'Number':[2]},{'Number':[3]},{'Number':[3]},{'Number':[1]},{'Number':[2]},{'Number':[3]},{'Number':[3]}])\
    ])

def testIncrement(addr, initial):
    return simulate(['set '+str(addr)+' '+str(initial)] + increment(addr) + copy(addr,5))

print("Problem #2, part (a), increment()...")
try: increment
except: print("The increment() function is not defined.")
else: check('testIncrement', testIncrement, [\
        ((9, 123), [124]),\
        ((20, 234), [235]),\
        ((100, 0), [1]),\
        ((1000, 1), [2])\
        ])

print("Problem #3, part (d), compile()...")
try: compile
except: print("The compile() function is not defined.")
else: check(('simulate(compile(', '))'), lambda s: simulate(compile(s)), [\
    (["print 123;"], [123]),\
    (["print false; print true; print 4;"], [0, 1, 4]),\
    (["x := 10; print x;"], [10]),\
    (["x := 10; print x + x;"], [20]),\
    (["x := 1; y := 2; z := 3; print x + y + z;"], [6]),\
    (["print true and false;"], [0]),\
    (["x := true xor false; print x;"], [1]),\
    (["x := true xor false; print x and not(x);"], [0]),\
    (["x := true; y := false; z := true; print (not(x) xor y) and z;"], [0]),\
    (["if true {print 4;}"], [4]),\
    (["if true {print true;} print false;"], [1,0]),\
    (["x := false; until not(x) {print false;} print true;"], [1]),\
    (["x := false; until x { x := true; } print false;"], [0]),\
    (["x := false; y := false; until x {print false; x := y and true; if not(y) {y := true;} } print true;"], [0, 0, 1]),\
    (["if true {print true;} print false;"], [1,0]),\
    (["procedure f { } print 4;"], [4]),\
    (["procedure example {print 4;} call example;"], [4]),\
    (["x := 123; procedure example {print x;} call example; call example;"], [123,123]),\
    (["procedure g {print 2;} procedure f {call g; print 1; call g;} call f;"], [2,1,2]),\
    (["procedure g {print 2;} if true and true { call g; }"], [2]),\
    (["procedure g {print 2;} procedure f {if true and true { call g; }} call g; call f;"], [2,2]),\
    (["procedure h {print 3;} procedure g {print 2; call h; call h;} procedure f {call g; print 1; call g;} call f;"], [2,3,3,1,2,3,3])\
    ])

#eof
