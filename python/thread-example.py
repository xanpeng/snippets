#-*- encoding: utf8 -*-
import threading
import time

class Test(threading.Thread):
    def __init__(self, num):
        threading.Thread.__init__(self)
        self._run_num = num

    def run(self):
        global count, mutex
        threadname = threading.currentThread().getName()
        for x in xrange(0, int(self._run_num)):
            mutex.acquire()
            count = count + 1
            mutex.release()
            print threadname, x, count
            time.sleep(1)

if __name__ == '__main__':
    global count, mutex
    count = 1
    mutex = threading.Lock()
    threads = []

    for x in xrange(0, 4):  # create child threads
        threads.append(Test(10))
    for t in threads:   # start child threads
        t.start()
    for t in threads:   # wait for child threads to die
        t.join()  
