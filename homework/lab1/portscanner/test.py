import re
import threading
import time
import threadpool

class scanthread(threading.Thread):
    _num = 0
    _pool = None
    _lock = threading.Lock()
    _is_running = False

    def __init__(self, num):
        threading.Thread.__init__(self)
        self._num = num
        self._pool = threadpool.ThreadPool(10)

    def run(self):
        self._lock.acquire()
        self._is_running = True
        self._lock.release()
        urls_list=[]
        for i in range(self._num):
            urls_list.append('hello '+str(i))
        reqs = threadpool.makeRequests(fun, urls_list)
        for req in reqs:
            # if self._is_running is False:
            #     self._pool.dismissWorkers(10, True)
            #     break
            self._pool.putRequest(req)
        try:
            self._pool.wait()
        except threadpool.NoWorkersAvailable:
            print('scan stopped...')
    
    def stop(self):
        self._lock.acquire()
        self._is_running = False
        self._lock.release()
        self._pool.dismissWorkers(10, True)

    
    def stopped(self):
        self._lock.acquire()
        re=(self._is_running is False)
        self._lock.release()
        return re


def fun(name):
    time.sleep(1)
    print(name)

if __name__ == "__main__":
    s = scanthread(1000)
    s.start()
    time.sleep(10)
    # time.sleep(1)
    s.stop()
    