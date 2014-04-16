#!env python
 
"""How to use: python list-modules.py [| grep argpase]
"""
import sys
from pkgutil import iter_modules

it = iter_modules()
while True:
    try: x = it.next()
    except: break
    print x[0], '\t', x[1]
