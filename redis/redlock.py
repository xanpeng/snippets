# coding=utf-8

import string
import random
import redis
import time
from collections import namedtuple

Lock = namedtuple('Lock', ('validity', 'resource', 'key'))


class CannotObtainLock(Exception):
    pass


class Redlock(object):
    default_retry_count = 3
    default_retry_delay = 0.2
    clock_drift_factor = 0.01
    unlock_script = """
    if redis.call("get",KEYS[1]) == ARGV[1] then
        return redis.call("del",KEYS[1])
    else
        return 0
    end"""

    def __init__(self, connection_list, retry_count=None, retry_delay=None):
        if not isinstance(connection_list, list):
            connection_list = [connection_list]
        self.servers = []
        for connection_info in connection_list:
            try:
                if isinstance(connection_info, str):
                    # redis://localhost:6379/0
                    redis_server = redis.StrictRedis.from_url(connection_info)
                else:
                    redis_server = redis.StrictRedis(**connection_info)
                self.servers.append(redis_server)
            except Exception as e:
                logging.warning('wrong connection info {0}'.format(connection_info))
                raise Warning(str(e))
        self.quorum = (len(connection_list) / 2) + 1

        if len(self.servers) < self.quorum:
            logging.warning('failed to connect to the majority of redis servers')
            raise CannotObtainLock('failed to connect to the majority of redis servers')
        self.retry_count = retry_count or self.default_retry_count
        self.retry_delay = retry_delay or self.default_retry_delay

    @classmethod
    def lock_instance(cls, server, resource, val, ttl):
        try:
            ret = server.set(resource, val, nx=True, px=ttl)
            logging.debug('lock {0}/{1} got {2}'.format(server, resource, ret))
            return ret
        except Exception as e:
            logging.debug('lock {0}/{1} got {2}'.format(server, resource, e))
            return False

    @classmethod
    def unlock_instance(cls, server, resource, val):
        try:
            server.eval(cls.unlock_script, 1, resource, val)
        except:
            pass

    @classmethod
    def get_unique_id(cls):
        chars = string.ascii_letters + string.digits
        return ''.join(random.choice(chars) for _ in range(22))

    def lock(self, resource, ttl):
        retry = 0
        val = self.get_unique_id()
        # Add 2 milliseconds to the drift to account for Redis expires
        # precision, which is 1 millisecond, plus 1 millisecond min
        # drift for small TTLs.
        drift = int(ttl * self.clock_drift_factor) + 2

        while retry < self.retry_count:
            n = 0
            start_time = int(time.time() * 1000)
            for server in self.servers:
                if self.lock_instance(server, resource, val, ttl):
                    n += 1
            elapsed_time = int(TimeUtils.now() * 1000) - start_time
            validity = int(ttl - elapsed_time - drift)
            if validity > 0 and n >= self.quorum:
                logging.debug('lock {0} succeed'.format(resource))
                return Lock(validity, resource, val)
            else:
                for server in self.servers:
                    self.unlock_instance(server, resource, val)
                logging.debug('lock retry {0}'.format(resource))
                retry += 1
                TimeUtils.sleep_sync(self.retry_delay)
        logging.debug('lock {0} failed'.format(resource))
        return False

    def unlock(self, lock):
        for server in self.servers:
            self.unlock_instance(server, lock.resource, lock.key)


def create_redlock():
    return Redlock([{'host': 'localhost'}])


def test_lock(dlm):
    assert isinstance(dlm, Redlock)
    lock = dlm.lock('pants', 100)
    assert lock.resource == 'pants'
    dlm.unlock(lock)
    lock = dlm.lock('pants', 10)
    dlm.unlock(lock)


def test_blocked(dlm):
    lock = dlm.lock('pants', 1000)
    bad = dlm.lock('pants', 10)
    assert bad is False
    dlm.unlock(lock)


def test_bad_connection_info():
    Redlock([{'cat': 'dog'}])


if __name__ == '__main__':
    dlm = create_redlock()
    test_lock(dlm)
    test_blocked(dlm)
    test_bad_connection_info()

