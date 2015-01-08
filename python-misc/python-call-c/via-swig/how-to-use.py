
# cos_module.so contains the compiled extension, which can be load in the IPython interpreter
#
# more at: https://scipy-lectures.github.io/advanced/interfacing_with_c/interfacing_with_c.html

import cos_module

if __name__ == '__main__':
    print cos_module
    print dir(cos_module)
    print cos_module.cos_func(1.0)
    print cos_module.cos_func(0.0)
    print cos_module.cos_func(3.1415926535)
    print cos_module.cos_func('foo')

