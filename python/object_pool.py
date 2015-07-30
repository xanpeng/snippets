# coding=utf-8

import Queue
import types
from contextlib import contextmanager


class ObjectPool(object):
    def __init__(self, fn_cls, *args, **kwargs):
        super(ObjectPool, self).__init__()
        self.fn_cls = fn_cls
        self.args = args
        self.max_size = int(kwargs.get('max_size', 1))
        self.queue = Queue.Queue(maxsize=self.max_size)

    def _create_obj(self):
        if isinstance(self.fn_cls, (types.FunctionType, types.MethodType)):
            return self.fn_cls(*self.args)
        elif isinstance(self.fn_cls, (types.ClassType, types.TypeType)):
            return apply(self.fn_cls, *self.args)
        else:
            raise Exception("Wrong type")

    def borrow_obj(self):
        if self.queue.qsize() < self.max_size and self.queue.empty():
            self.queue.put(self._create_obj())
        return self.queue.get()

    def recycle_obj(self, obj):
        self.queue.put(obj)

    def count(self):
        return self.queue.qsize()

    @contextmanager
    def get(self):
        obj = self.borrow_obj()
        try:
            yield obj
        except Exception:
            yield None
        finally:
            self.recycle_obj(obj)


if __name__ == '__main__':
    class TestClass(object):
        @classmethod
        def create(cls):
            instance = TestClass()
            print 'TestClass.create {0}'.format(instance)
            return instance

    instance_pool = ObjectPool(TestClass.create, max_size=4)

    import threading
    class MyThread(threading.Thread):
        def run(self):
            with instance_pool.get() as instance:
                print 'pool total count {0}, current is {1}'.format(instance_pool.count(), instance)

    threads = []
    for i in xrange(100):
        t = MyThread()
        t.start()
        threads.append(t)
    for t in threads:
        t.join(True)

