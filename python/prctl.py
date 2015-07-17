# coding=utf-8

"""
intro:
1) ipc
2) process creation/operation.
3) daemon creation/operation.
4) process control, like set process name.
5) ...
"""
import os
import sys
import atexit
import signal
import subprocess
import multiprocessing
import threading
import gevent
import logging

from . import _prctl  # c interface


# create _prctl wrapper
def prctl_wrapper(option):
    def call_prctl(arg=0):
        return _prctl.prctl(option, arg)
    return call_prctl

set_name = prctl_wrapper(getattr(_prctl, 'PR_SET_NAME'))
get_name = prctl_wrapper(getattr(_prctl, 'PR_GET_NAME'))
set_proctitle = _prctl.set_proctitle
del prctl_wrapper


def current_pid():
    return multiprocessing.current_process().pid


def read_pid(pidfile):
    try:
        f = file(pidfile, 'r')
        pid = int(f.read().strip())
        f.close()
    except IOError:
        pid = None
    logging.debug('read_pid({0})={1}'.format(pidfile, pid))
    return pid


def write_pid(pidfile, pid):
    """write pid into pidfile"""
    try:
        with open(pidfile, 'w+') as f:
            f.write(str(int(pid)))
    except IOError:
        logging.warning('write_pid({0}, {1}) failed'.format(pidfile, pid))
        raise


def delete_pidfile(pidfile):
    try:
        os.remove(pidfile)
    except (OSError, IOError):
        pass


def sigkill(pidfile, sig=signal.SIGINT):
    """send a SIGKILL to process with pid in pidfile."""
    pid = read_pid(pidfile)
    if pid is None:
        logging.debug('read_pid({0}) failed'.format(pidfile))
        return None
    try:
        os.kill(pid, sig)
    except OSError as err:
        logging.error(str(err))
    finally:
        if not is_pid_alive(pid):
            logging.debug('remove {0}'.format(pidfile))
            try:
                os.remove(pidfile)
            except OSError:
                pass
    return pid


def is_pid_alive(pid):
    """检查对应进程是否运行"""
    if not isinstance(pid, (int, long)):
        return False
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    else:
        logging.debug('{0} is running'.format(pid))
        return True


def is_process_running(procname, pidfile):
    """
    pidfile存在，检查进程是否存在；
    pidfile不存在，仍检查进程是否存在；
    :return: True表示存在运行中进程，False表示不存在
    """
    pid = read_pid(pidfile)
    if pid is None and procname is None:
        return False

    if procname:
        cmdline = 'pgrep %s' % procname
        proc = subprocess.Popen(cmdline.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, _ = proc.communicate()
        try:
            proc.kill()
        except OSError:
            pass

        logging.debug('"{0}" got "{1}"'.format(cmdline, out.split()))
        if len(out.split()) > 0:
            return True
        return False
    else:
        return is_pid_alive(pid)


def spawn_and_record(procname, pidfile, sighandler, func, *args):
    def record_process(pid):
        piddir = os.path.dirname(pidfile)
        if not os.path.exists(piddir):
            os.makedirs(piddir)
        with open(pidfile, 'w') as pf:
            pf.write(str(pid))

    # requires len(name) <= 10
    class _Wrapper(multiprocessing.Process):
        def run(self):
            set_proctitle(procname)   # ps -ef
            set_name(procname)        # pgrep + pkill
            install_signal_handler(sighandler)
            multiprocessing.Process.run(self)
    proc = _Wrapper(target=func, args=args)
    proc.name = procname
    proc.daemon = False
    proc.start()
    record_process(proc.pid)
    return proc


def run_as_process(target, args=(), kwargs=None):
    p = multiprocessing.Process(target=target, args=args, kwargs=kwargs)
    p.daemon = True
    p.start()


def run_as_thread(target, args=(), kwargs=None):
    t = threading.Thread(target=target, args=args, kwargs=kwargs)
    t.start()


def run_as_greenlet(target, *args, **kwargs):
    gevent.spawn(target, *args, **kwargs)


def signal_name(signum):
    """
    {1: 'SIGHUP', 2: 'SIGINT', 3: 'SIGQUIT', 4: 'SIGILL', 5: 'SIGTRAP', 6: 'SIGIOT',
    7: 'SIGBUS', 8: 'SIGFPE', 9: 'SIGKILL', 10: 'SIGUSR1', 11: 'SIGSEGV', 12: 'SIGUSR2',
    13: 'SIGPIPE', 14: 'SIGALRM', 15: 'SIGTERM', 17: 'SIGCLD', 18: 'SIGCONT',
    19: 'SIGSTOP', 20: 'SIGTSTP', 21: 'SIGTTIN', 22: 'SIGTTOU', 23: 'SIGURG',
    24: 'SIGXCPU', 25: 'SIGXFSZ', 26: 'SIGVTALRM', 27: 'SIGPROF', 28: 'SIGWINCH',
    29: 'SIGPOLL', 30: 'SIGPWR', 31: 'SIGSYS', 34: 'SIGRTMIN', 64: 'SIGRTMAX'}
    """
    _signames = dict((getattr(signal, signame), signame)
                     for signame in dir(signal)
                     if signame.startswith('SIG') and '_' not in signame)
    try:
        return _signames[signum]
    except KeyError:
        return 'SIG_UNKNOWN'


def install_signal_handler(handler=None):
    def noop_handler(signum, frame):
        logging.debug('got signal {0}:{1}'.format(signal_name(signum), frame))
        raise SystemExit

    if handler is None:
        signal.signal(signal.SIGINT, noop_handler)
        signal.signal(signal.SIGTERM, noop_handler)
    else:
        signal.signal(signal.SIGINT, handler)
        signal.signal(signal.SIGTERM, handler)


class Daemon(object):
    def __init__(self, pidfile, logfile='/dev/null', stdin='/dev/null'):
        logfile = logfile or '/dev/null'
        self.stdin = stdin
        self.stdout = logfile
        self.stderr = logfile
        self.pidfile = pidfile

    def set_pidfile(self, pidfile):
        self.pidfile = pidfile

    def set_logfile(self, logfile):
        self.stderr = logfile
        self.stdout = logfile

    def delpid(self):
        os.remove(self.pidfile)

    def getpid(self):
        try:
            pidfile = file(self.pidfile, 'r')
            pid = int(pidfile.read().strip())
            pidfile.close()
            return pid
        except IOError:
            return '-1'

    def _daemonize(self):
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)  # exit from first parent
        except OSError, e:
            sys.stderr.write('fork #1 failed: %d (%s)\n' % (e.errno, e.strerror))
            sys.exit(1)

        # decouple from parent environment
        os.chdir('/')
        os.setsid()
        os.umask(0)

        try:
            pid = os.fork()  # do second fork
            if pid > 0:
                sys.exit(0)  # exit from second parent
        except OSError, e:
            sys.stderr.write('fork #2 failed: %d (%s)\n' % (e.errno, e.strerror))
            sys.exit(1)

        # redirect standard fd
        sys.stdout.flush()
        sys.stderr.flush()
        si = file(self.stdin, 'r')
        so = file(self.stdout, 'a+')
        se = file(self.stderr, 'a+', 0)
        os.dup2(si.fileno(), sys.stdin.fileno())
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

        # write pidfile
        atexit.register(self.delpid)
        pid = str(os.getpid())
        file(self.pidfile, 'w+').write('%s\n' % pid)

    def _daemonize_and_run(self):
        self._daemonize()
        self.run()

    def _daemonize_and_keep_caller(self):
        try:
            agent = multiprocessing.Process(target=self._daemonize_and_run)
            agent.start()
            # main process will keep running
        except OSError, e:
            sys.stderr.write('fork agent failed: %d (%s)\n' % (e.errno, e.strerror))
            sys.exit(1)

    def start(self):
        """start the daemon"""
        try:
            pidfile = file(self.pidfile, 'r')
            pid = int(pidfile.read().strip())
            pidfile.close()
        except IOError:
            pid = None

        if pid:
            sys.stderr.write('pidfile %s already exist. Daemon already running?\n' % self.pidfile)
            sys.exit(1)

        self._daemonize_and_keep_caller()

    def stop(self):
        """stop the daemon"""
        try:
            pidfile = file(self.pidfile, 'r')
            pid = int(pidfile.read().strip())
            pidfile.close()
        except IOError:
            pid = None

        if not pid:
            sys.stderr.write('pidfile %s does not exist. Daemon not running?\n' % self.pidfile)
            return  # not an error in a restart

        try:
            os.kill(pid, signal.SIGINT)
        except OSError as err:
            err = str(err)
            if err.find('No such process') > 0:
                if os.path.exists(self.pidfile):
                    os.remove(self.pidfile)
            else:
                sys.stderr.write(err)
                sys.exit(1)

    def restart(self):
        self.stop()
        self.start()

    def run(self):
        """override this method in your subclass"""
        raise NotImplementedError


def daemonize_myself(pidfile, logfile):
    stdin = '/dev/null'
    stdout = logfile
    stderr = logfile

    def _delpid():
        logging.debug('process with pidfile "{0}" exits'.format(pidfile))
        delete_pidfile(pidfile)

    def daemonize():
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)  # exit from first parent
        except OSError, e:
            sys.stderr.write('fork #1 failed: %d (%s)\n' % (e.errno, e.strerror))
            sys.exit(1)

        # decouple from parent environment
        os.chdir('/')
        os.setsid()
        os.umask(0)

        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)  # exit from second parent
        except OSError, e:
            sys.stderr.write('fork #2 failed: %d (%s)\n' % (e.errno, e.strerror))
            sys.exit(1)

        # redirect standard fd
        sys.stdout.flush()
        sys.stderr.flush()
        si = file(stdin, 'r')
        so = file(stdout, 'a+')
        se = file(stderr, 'a+', 0)
        os.dup2(si.fileno(), sys.stdin.fileno())
        os.dup2(so.fileno(), sys.stdout.fileno())
        os.dup2(se.fileno(), sys.stderr.fileno())

        pid = str(os.getpid())
        file(pidfile, 'w+').write('%s\n' % pid)

        atexit.register(_delpid)

        return pid

    pid_ret = daemonize()
    return pid_ret


def test_daemonize_myself():
    import tempfile
    print('before call ipc.daemonize_myself, I am {0}'.format(ipctools.current_pid()))
    pidfile, logfile = tempfile.mktemp(), '/tmp/testdaemon.log'
    ipctools.daemonize_myself(pidfile, logfile)
    print('after ipc.daemonize_myself')  # this will be written to logfile


def daemonize_cmdline(cmdline, pidfile, logfile=None):
    """make cmdline a background-running daemon"""
    class _CmdlineDaemon(Daemon):
        def __init__(self):
            super(_CmdlineDaemon, self).__init__(pidfile, logfile)

        def run(self):  # need list
            proc = subprocess.Popen(cmdline)
            file(self.pidfile, 'w+').write('%s\n' % proc.pid)
            proc.wait()

    cmdline_daemon = _CmdlineDaemon()
    cmdline_daemon.start()
