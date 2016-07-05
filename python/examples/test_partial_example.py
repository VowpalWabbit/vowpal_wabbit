from vowpalwabbit import pyvw

vw = pyvw.vw('--audit')
full = vw.example( { 'a': ['b'], 'x': ['y'] } )
full.learn()

part = vw.example( {'a': ['b'] } )
part.learn()

part.push_features('x', ['y'])
part.learn()

part.erase_namespace(ord('x'))
part.push_features('x', ['z'])
part.learn()
