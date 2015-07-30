# coding=utf-8

# based on rq: http://python-rq.org/

import uuid
import sys
import signal
import inspect
import functools
import traceback
import importlib
import pickle

TASK_KEYPREFIX = 'task:'
QUEUES_KEYS = 'queues'
QUEUE_READY_KEYPREFIX = 'queue:ready:'
QUEUE_WIP_KEYPREFIX = 'queue:wip:'
QUEUE_FINISHED_KEYPREFIX = 'queue:finished:'
QUEUE_FAILED = 'queue:failed'
SORTEDSET_DEFERRED_KYEPREFIX = 'deferred:'
TIMEOUT_READ_REDIS = 15


class StringUtils(object):
    @staticmethod
    def as_text(v):
        if v is None:
            return None
        return v.decode('utf-8')


class Stream(object):
    @classmethod
    def to_bytes(cls, obj):
        try:
            return pickle.dumps(obj, protocol=pickle.HIGHEST_PROTOCOL)
        except Exception:
            return None

    @classmethod
    def from_bytes(cls, serialized):
        try:
            return pickle.loads(serialized)
        except Exception:
            return None


class TimeUtils(object):
    @classmethod
    def sleep_sync(cls, seconds):
        time.sleep(seconds)

    @classmethod
    def sleep_co(cls, seconds):
        gevent.sleep(seconds)

    @classmethod
    def now(cls):
        return time.time()

    @staticmethod
    def utcnow():
        return datetime.datetime.utcnow()

    @staticmethod
    def utcformat(dt):
        return dt.strftime(StringUtils.as_text('%Y-%m-%dT%H:%M:%SZ'))

    @staticmethod
    def utcparse(string):
        try:
            return datetime.datetime.strptime(string, '%Y-%m-%dT%H:%M:%SZ')
        except ValueError:
            return datetime.datetime.strptime(string, '%Y-%m-%dT%H:%M:%S.%f+00:00')

    @staticmethod
    def current_timestamp():
        return calendar.timegm(datetime.datetime.utcnow().utctimetuple())


class Task(object):
    @classmethod
    def gen_signature(cls, func, kwargs):
        func_name, _ = cls._process_func(func)
        signature = '%s(%s)' % (func_name, kwargs or {})
        logging.debug('got {0}'.format(signature))
        return signature

    class Status(object):
        READY = 'ready'
        RUNNING = 'wip'
        FINISHED = 'finished'
        FAILED = 'failed'

    class Result(object):
        SUCCEED = 'taskresult:succeed'
        FAILED = 'taskresult:failed'

    @classmethod
    def fetch(cls, task_id):
        task = Task()
        task.id = task_id
        obj = redis.hgetall(task.gen_key())
        if len(obj) == 0:
            logging.info('{0} not found'.format(task_id))
            return None
        try:
            task.data = obj['data']
        except KeyError:
            logging.warning('{0} has no <data> field'.format(obj))
            return None
        task.func_name, task.instance, task.kwargs = Stream.from_bytes(task.data)

        def to_date(date_str):  # 初始化是"None"字符串
            return None if date_str == 'None' else TimeUtils.utcparse(StringUtils.as_text(date_str))

        task.signature = obj.get('signature')
        task.created_at = to_date(StringUtils.as_text(obj.get('created_at')))
        task.enqueued_at = to_date(StringUtils.as_text(obj.get('enqueued_at')))
        task.ended_at = to_date(StringUtils.as_text(obj.get('ended_at')))
        task.origin = StringUtils.as_text(obj.get('origin'))
        task.result = Stream.from_bytes(obj.get('result')) if obj.get('result') else None
        task.exc_info = obj.get('exc_info')
        task.status = StringUtils.as_text(obj.get('status') if obj.get('status') else None)
        task.meta = Stream.from_bytes(obj.get('meta')) if obj.get('meta') else {}
        return task

    @classmethod
    def create(cls, func, kwargs):
        task = Task()
        task.id = unicode(uuid.uuid4())
        task.func_name, task.instance = cls._process_func(func)
        task.kwargs = kwargs or {}
        task.data = Stream.to_bytes((task.func_name, task.instance, task.kwargs))
        task.signature = task.gen_signature(func, kwargs=task.kwargs)
        task.created_at, task.enqueued_at, task.ended_at = TimeUtils.utcnow(), None, None
        task.origin = None
        task.exc_info = None
        task.meta = {}
        task.result = None
        task.status = None
        return task

    @classmethod
    def _process_func(cls, func):
        if inspect.ismethod(func):
            func_name, instance = func.__name__, func.__self__
        elif inspect.isfunction(func) or inspect.isbuiltin(func):
            func_name, instance = '%s.%s' % (func.__module__, func.__name__), None
        else:
            logging.debug('func "{0}" is not callable so use str(func)'.format(func))
            func_name, instance = str(func), None
        return func_name, instance

    def __init__(self):
        self.id = None
        self.func_name = None
        self.instance = None
        self.kwargs = None
        self.data = None
        self.signature = None
        self.created_at = None
        self.enqueued_at = None
        self.ended_at = None
        self.origin = None
        self.result = None
        self.exc_info = None
        self.status = None
        self.meta = {}

    def gen_key(self):
        return '{0}{1}'.format(TASK_KEYPREFIX, self.id.encode('utf-8'))

    def fetch_result(self):
        if self.result is None:
            rv = redis.hget(self.gen_key(), 'result')
            if rv is not None:
                self.result = Stream.to_bytes(rv)
        return self.result

    def set_status(self, status):
        self.status = str(status)
        redis.hset(self.gen_key(), 'status', self.status)

    def save(self):
        obj = {'created_at': TimeUtils.utcformat(self.created_at or TimeUtils.utcnow()),
               'enqueued_at': TimeUtils.utcformat(self.enqueued_at) if self.enqueued_at else 'None',
               'ended_at': TimeUtils.utcformat(self.ended_at) if self.ended_at else 'None',
               'data': self.data,
               'origin': self.origin,
               'signature': self.signature,
               'result': Stream.to_bytes(self.result),
               'exc_info': self.exc_info,
               'status': self.status,
               'meta': Stream.to_bytes(self.meta)}
        redis.hmset(self.gen_key(), obj)

    def perform(self):
        """execute task in sync manner"""
        logging.info('do task {0}'.format(self.id))
        if self.instance:
            func = getattr(self.instance, self.func_name)
        else:
            # Get an attribute from a dotted path name (e.g. "path.to.func").
            module_name, attribute = self.func_name.rsplit('.', 1)
            module = importlib.import_module(module_name)
            func = getattr(module, attribute)
        self.result = func(**self.kwargs)
        return self.result

    def __str__(self):
        return '<task %s: %s>' % (self.id, self.signature)

    def __repr__(self):
        return 'task(%r, enqueued_at=%r)' % (self.id, self.enqueued_at)

    def __eq__(self, other):
        return self.id == other.id

    def __hash__(self):
        return hash(self.id)


class TaskBookkeep(object):
    """4种状态：ready（隐含）、running、finished、failed

    ready -> running
    running -> finished/failed
    """
    # 根据Task的执行结果，判定Task的下一个状态
    TASK_STATUS_TRANSITION = {
        Task.Result.SUCCEED: Task.Status.FINISHED,
        Task.Result.FAILED: Task.Status.FAILED,
    }

    @classmethod
    def next_status(cls, result):
        if result in [Task.Result.SUCCEED, Task.Result.FAILED]:
            return cls.TASK_STATUS_TRANSITION[result]
        return cls.TASK_STATUS_TRANSITION[Task.Result.SUCCEED]

    class BaseRegistry(object):
        def __init__(self, queue_name):
            self.name = queue_name
            self.key = None

        def add(self, task, ttl=-1):
            score = ttl if ttl < 0 else TimeUtils.current_timestamp() + ttl
            ret = redis.zadd(self.key, score, task.id)
            return ret

        def remove(self, task):
            ret = redis.zrem(self.key, task.id)
            return ret

    class FailedTaskRegistry(BaseRegistry):
        """queue:failed"""
        def __init__(self, queue_name):
            super(TaskBookkeep.FailedTaskRegistry, self).__init__(queue_name)
            self.key = QUEUE_FAILED

    class FinishedTaskRegistry(BaseRegistry):
        """queue:finished:xxx"""
        def __init__(self, queue_name):
            super(TaskBookkeep.FinishedTaskRegistry, self).__init__(queue_name)
            self.key = QUEUE_FINISHED_KEYPREFIX + queue_name

    class RunningTaskRegistry(BaseRegistry):
        """queue:wip:xxx"""
        def __init__(self, queue_name):
            super(TaskBookkeep.RunningTaskRegistry, self).__init__(queue_name)
            self.key = QUEUE_WIP_KEYPREFIX + queue_name

    def __init__(self, queue_name):
        # Task在不同阶段，放在下面的不同子队列中
        self.registries = {
            Task.Status.FINISHED: self.FinishedTaskRegistry(queue_name),
            Task.Status.FAILED: self.FailedTaskRegistry(queue_name),
            Task.Status.READY: None,
            Task.Status.RUNNING: self.RunningTaskRegistry(queue_name), }

    def bookkeep_task(self, task, from_status, to_status):
        """
        :param from_status: Task.Status.XXX
        """
        logging.info('bookkeep from {0} to {1}'.format(from_status, to_status))
        self.registries[to_status].add(task)
        task.set_status(to_status)
        if from_status != Task.Status.READY:  # ready队列Queue.dequeue_any()会去更新
            task.ended_at = TimeUtils.utcnow()
            self.registries[from_status].remove(task)
        task.save()

    def bookkeep_failed_task(self, task, *exc_info):
        exc_string = ''.join(
            traceback.format_exception_only(*exc_info[:2]) + traceback.format_exception(*exc_info))
        logging.error(exc_string, exc_info=True, extra={
            'func': task.func_name, 'kwargs': task.kwargs, 'queue': task.origin,})
        exc_string = ''.join(traceback.format_exception(*exc_info))
        task.exc_info = exc_string
        logging.info('move task to {0} queue'.format(QUEUE_FAILED))
        self.bookkeep_task(task, from_status=Task.Status.RUNNING, to_status=Task.Status.FAILED)


class Queue(object):
    """Queue包含多个状态，分别用redis list表示。"""
    task_class = Task

    @classmethod
    def make_full_queue_key(cls, raw_name):
        return QUEUE_READY_KEYPREFIX + raw_name

    @classmethod
    def from_queue_key(cls, key):
        if not key.startswith(QUEUE_READY_KEYPREFIX):
            logging.info('{0} is not a valid queue key'.format(key))
            return None
        return cls(key[len(QUEUE_READY_KEYPREFIX):])

    @classmethod
    def dequeue_any(cls, queues):
        """从queue:ready:[名字]中获取task. """
        queues = queues or []
        queue_keys = [q.key for q in queues]
        logging.debug('dequeue from {0}'.format(queue_keys))
        if len(queues) == 0:
            return None, None

        queue_key, task_id = cls.lpop(queue_keys, timeout=TIMEOUT_READ_REDIS)
        if queue_key is None:
            return None, None

        queue, task = cls.from_queue_key(queue_key), cls.task_class.fetch(task_id)
        if task is None:
            logging.debug('read task failed and dequeue_any again')
            return cls.dequeue_any(queues)
        return task, queue

    @classmethod
    def lpop(cls, queue_keys, timeout=None):
        if timeout is not None:  # blocking方式
            result = redis.blpop(queue_keys, timeout)  # redis blpop返回(queue key, id)
            if result is None:
                logging.debug('blpop dequeue {0} timeout'.format(queue_keys))
                return None, None
            queue_key, task_id = result
            return StringUtils.as_text(queue_key), StringUtils.as_text(task_id)
        else:  # non-blocking方式
            for queue_key in queue_keys:
                task_id = redis.lpop(queue_key)
                if task_id is not None:
                    return StringUtils.as_text(queue_key), StringUtils.as_text(task_id)
            return None, None

    def __init__(self, name, async=True):
        self.name = name
        self.key = '%s%s' % (QUEUE_READY_KEYPREFIX, name)
        self.async = async

    def enqueue_task(self, task, at_front=False):
        logging.info('enqueue {0}'.format(task))
        redis.sadd(QUEUES_KEYS, self.key)
        task.set_status(Task.Status.READY)
        task.origin, task.enqueued_at = self.name, TimeUtils.utcnow()
        task.save()

        if self.async:
            redis.lpush(self.key, task.id) if at_front else redis.rpush(self.key, task.id)
        else:
            task.perform()
            task.save()
        return task

    def __len__(self):
        return redis.llen(self.key)

    def __hash__(self):
        return hash(self.name)

    def __repr__(self):
        return 'Queue(%r)' % (self.name,)

    def __str__(self):
        return '<Queue \'%s\'>' % (self.name,)


class DeferredQueue(Queue):
    """包含延时功能的队列，延时的任务先记录在deferred:[队列名]集合中"""
    RETRY_INTERVAL = 60

    @classmethod
    def dequeue_any(cls, queues):
        queues = queues or []
        queue_keys = [q.key for q in queues]
        logging.debug('dequeue from {0}'.format(queue_keys))
        if len(queues) == 0:
            return None, None

        queue_key, task_id = cls.lpop(queue_keys, timeout=None)
        # queue中有待执行的task
        if queue_key is not None and task_id is not None:
            queue, task = cls.from_queue_key(queue_key), cls.task_class.fetch(task_id)
            if task is None:
                logging.debug('read task failed and dequeue_any again')
                return cls.dequeue_any(queues)
            logging.info('got {0}.{1}'.format(queue_key, task))
            return task, queue
        # queue中没有task，查找延时任务集合deferred:[队列名]
        else:
            while True:
                has_deferred = False
                for q in queues:
                    timeup_cnt = cls._handle_timeups(cls._gen_deferred_key(q.name), q)
                    has_deferred = has_deferred or bool(timeup_cnt > 0)
                if has_deferred:
                    break
                logging.debug('find nothing and sleep_co({0})+try'.format(cls.RETRY_INTERVAL))
                TimeUtils.sleep_co(cls.RETRY_INTERVAL)
            logging.debug('find deferred+timeup tasks')
            return cls.dequeue_any(queues)

    @classmethod
    def _handle_timeups(cls, deferred_key, related_queue):
        """检查deferred:[队列名]，将完成等待的task插入queue

        :return time-up的延时任务的数量
        """
        logging.debug('search {0}'.format(deferred_key))

        if not redis.exists(deferred_key):
            return 0
        raw_records = redis.zrangebyscore(deferred_key, 0, cls._now())
        removed = redis.zremrangebyscore(deferred_key, 0, cls._now())
        for serialized_record in raw_records:
            record = Stream.from_bytes(serialized_record)
            logging.debug('handle deferred {0}.{1}'.format(deferred_key, record))
            related_queue.enqueue_task(Task.create(record['func'], kwargs=record['kwargs']))
        logging.debug('remove %d time-up tasks' % removed)
        return len(raw_records)

    @classmethod
    def _now(cls):
        return int(TimeUtils.now())

    @classmethod
    def _gen_deferred_key(cls, raw_name):
        return SORTEDSET_DEFERRED_KYEPREFIX + raw_name

    def __init__(self, name, async=True):
        super(DeferredQueue, self).__init__(name, async)
        self.deferred_tasks_key = self._gen_deferred_key(name)

    def defer(self, seconds, func, kwargs):
        # thread-safe
        defer_to = self._now() + seconds
        ret = redis.zadd(
            self.deferred_tasks_key, defer_to, Stream.to_bytes({'func': func, 'kwargs': kwargs}))
        logging.info('delay {0} by {1}s got {2}'.format(func.__name__, seconds, ret))


class Worker(object):
    queue_class = Queue
    task_class = Task

    def __init__(self, name, queues):
        self.queues = queues  # Queue列表
        self.name = name
        self.stopped = False

    def _install_signal_handlers(self):
        def request_stop(signum, frame):
            self.stopped = True

        signal.signal(signal.SIGINT, request_stop)
        signal.signal(signal.SIGTERM, request_stop)

    def work(self, *args, **kwargs):
        """启动worker的work loop"""
        # self._install_signal_handlers()
        did_perform_work = False
        logging.info('worker.{0} running with {1}:{2}'.format(self.name, args, kwargs))
        while True:
            if self.stopped:
                logging.info('worker.{0} stopping on request'.format(self.name))
                break
            task, queue = self.dequeue_task()
            if task is None and queue is None:
                logging.warning('worker.{0} quit on null task'.format(self.name))
                break
            self.perform_task(task)
            did_perform_work = True
        return did_perform_work

    def dequeue_task(self):
        """
        :return (Task, Queue)/(None, None)
        """
        qnames = [q.name for q in self.queues]
        logging.debug('worker.{0}.dequeue_task from {1}'.format(self.name, ', '.join(qnames)))

        while True:
            if self.stopped:
                break
            # Queue中有timeout设置，DeferredQueue中有sleep重试
            task, queue = self.queue_class.dequeue_any(self.queues)
            if task is not None and queue is not None:
                logging.info('worker.{0}.dequeue_task got {1}'.format(self.name, task.signature))
                return task, queue
        return None, None

    def perform_task(self, task):
        """
        :return: True/False
        """
        logging.info('worker.{0}.perform_task {1}'.format(self.name, task))
        task_bookkeep = TaskBookkeep(task.origin)
        task_bookkeep.bookkeep_task(task, Task.Status.READY, Task.Status.RUNNING)
        try:
            rv = task.perform()
            # task.result有两种：一种是task函数的计算结果，另一种是Task.Result。
            # 如果使用前者，则默认是Task.Result.SUCCEED
            task.result = rv
            task_bookkeep.bookkeep_task(
                task, Task.Status.RUNNING, to_status=TaskBookkeep.next_status([task.result]))
        except Exception:
            task_bookkeep.bookkeep_failed_task(task, *sys.exc_info())
            return False
        return True


class DeferredWorker(Worker):
    queue_class = DeferredQueue

