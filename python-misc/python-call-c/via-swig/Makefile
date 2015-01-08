python_include=`python-config --cflags`

all: pythonway

swig:
	swig -python cos_module.i

# have problem in Mac
cway: swig
	gcc $(python_include) -c -fPIC cos_module_wrap.c cos_module.c
	ld cos_module_wrap.o cos_module.o -shared -o _cos_module.so

pythonway:
	python setup.py build_ext --inplace

clean:
	rm -rf build/
	rm -rf *.o *.so
	rm -rf cos_module.py cos_module_wrap.c
