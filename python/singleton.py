# coding=utf-8

import logging
logging.basicConfig()
logging.getLogger().setLevel(logging.DEBUG)
import threading
import random
import uuid


class Singleton(type):
    _singleton_lock = threading.RLock()
    """
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            with cls._singleton_lock:
                if cls not in cls._instances:
                    cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]
    """

    def __init__(cls, name, bases, attrs):
        super(Singleton, cls).__init__(name, bases, attrs)
        cls.instance = None

    def __call__(cls, *args, **kwargs):
        if cls.instance is None:
            with cls._singleton_lock:
                if cls.instance is None:
                    cls.instance = super(Singleton, cls).__call__(*args, **kwargs)
        return cls.instance


def test_singleton():
    def create_same_two(cls):
        instance_a, instance_b = cls(), cls()
        logging.debug('{0}: {1}, {2}'.format(cls, instance_a, instance_b))
        assert instance_a is instance_b

    def create_diff_two(clsa, clsb):
        instance_a, instance_b = clsa(), clsb()
        logging.debug('{0}: {1}, {2}: {3}'.format(clsa, instance_a, clsb, instance_b))
        assert instance_a is not instance_b

    # class Foo1(Singleton): pass  # 错误的写法（解释器报错），这样Foo1变成type了

    class Foo2(object):
        __metaclass__ = Singleton

    class Foo3(object):
        __metaclass__ = Singleton

    class Foo4(object):
        __metaclass__ = Singleton

        def __init__(self):
            self.msg = 'Foo4'

        def __str__(self):
            return '{0}: {1}'.format(id(self), self.msg)

    class Foo5(object):
        __metaclass__ = Singleton

        def __init__(self, inta, strb, tuplec):
            self.inta = inta
            self.strb = strb
            self.tuplec = tuplec

        def __str__(self):
            return '{0}: {1}, {2}, {3}'.format(id(self), self.inta, self.strb, self.tuplec)

    create_same_two(Foo2)
    create_diff_two(Foo2, Foo3)
    create_same_two(Foo4)

    # foo5_b的构造是无效的
    foo5_a, foo5_b = Foo5(1, 'foo5_a', (1, 'foo5_a_tuple')), Foo5(2, 'foo5_b', (2, 'foo5_b_tuple'))
    logging.debug('{0}: {1}, {2}'.format(Foo5, foo5_a, foo5_b))
    assert foo5_a is foo5_b

    instance_set = set()
    threads = []

    def create_foo5():
        instance_set.add(Foo5(
            random.randint(1, 10000), str(uuid.uuid4()), (random.randint(1, 100), str(uuid.uuid4()))))

    for i in xrange(1024):
        t = threading.Thread(target=create_foo5)
        threads.append(t)
        t.start()
    for t in threads:
        t.join()

    logging.debug('threads: {0}, Foo5 instances: {1}'.format(len(threads), len(instance_set)))
    assert len(threads) == 1024
    assert len(instance_set) == 1


if __name__ == '__main__':
    test_singleton()

