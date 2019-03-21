import tkinter as tk
import threading
import threadpool
import socket
import re

class scanthread(threading.Thread):
    _pool = None
    _texts=None
    _thread_n=None
    _ip_s=None
    _ip_e = None
    _port_s = None
    _port_e = None
    _stop_event = None
    _is_running = False

    def __init__(self, inputs, texts):
        threading.Thread.__init__(self)
        self._texts = texts
        self.get_params(inputs)
        self._stop_event = threading.Event()

    def run(self):
        self._is_running = True
        urls_list=[]
        tmp=[self._texts]
        index=0
        for i in range(self._ip_s, self._ip_e+1, 1):
            for j in range(self._port_s, self._port_e+1, 1):
                tmp.append((dec2addr(i), j))
                index+=1
                if index==2:
                    urls_list.append(tmp)
                    tmp=[self._texts]
                    index = 0
        self._texts.insert(tk.END, 'start scanning...\n')
        reqs = threadpool.makeRequests(scan_thread, urls_list)
        for req in reqs:
            self._pool.putRequest(req)
        try:
            self._pool.joinAllDismissedWorkers()
            self._pool.wait()
        except threadpool.NoWorkersAvailable:
            print('scan stopped...')
    
    def get_params(self, inputs):
        check_inputs(inputs, self._texts)
        self._ip_s = addr2dec(inputs['ip_start'].get())
        self._ip_e = addr2dec(inputs['ip_end'].get())
        self._port_s = int(inputs['port_start'].get())
        self._port_e = int(inputs['port_end'].get())
        self._thread_n = int(inputs['thread_num'].get())
        self._pool = threadpool.ThreadPool(self._thread_n)
    
    def stop(self):
        self._is_running = False
        self._pool.dismissWorkers(self._thread_n)
    
    def stopped(self):
        return self._is_running is False
        

scanner = None

def addr2dec(addr):
    """
    将点分十进制转换为整数
    """
    items = [int(x) for x in addr.split('.')]
    return sum([items[i] << [24, 16, 8, 0][i] for i in range(4)])


def dec2addr(dec):
    """
    将十进制IP转换为点分十进制
    """
    return '.'.join([str(dec>>x & 0xff) for x in [24, 16, 8, 0]])

def create_window():
    window = tk.Tk()
    window.title('端口扫描器')
    window.geometry('600x600')
    window.resizable(False, False)
    input_area = tk.Frame(window, height=200, width=600, bg='red')
    show_area = tk.Frame(window, height=400, width=600, bg='green')
    ip_range = tk.Frame(input_area, height=70, width=600)
    port_range = tk.Frame(input_area, height=70, width=600)
    button_area = tk.Frame(input_area, height=60, width=600)
    input_area.pack()
    show_area.pack()
    ip_range.pack()
    port_range.pack()
    button_area.pack()

    ip_start = tk.StringVar(value='127.0.0.1')
    ip_end = tk.StringVar(value='127.0.0.2')
    label1 = tk.Label(ip_range, text='输入 ip 范围：')
    label_curv1 = tk.Label(ip_range, text='~')
    ip_entry1 = tk.Entry(ip_range, width=25, textvariable=ip_start)
    ip_entry2 = tk.Entry(ip_range, width=25, textvariable=ip_end)

    label1.place(x=80, rely = 0.5, anchor=tk.CENTER)
    ip_entry1.place(x=210, rely=0.5, anchor=tk.CENTER)
    label_curv1.place(x=320, rely = 0.5, anchor=tk.CENTER)
    ip_entry2.place(x=430, rely=0.5, anchor=tk.CENTER)
    
    port_start = tk.StringVar(value='233')
    port_end = tk.StringVar(value='333')
    thread_num = tk.StringVar(value='10')
    label2 = tk.Label(port_range, text='输入端口范围：')
    port_entry1 = tk.Entry(port_range, width=12, textvariable=port_start)
    port_entry2 = tk.Entry(port_range, width=12, textvariable=port_end)
    label_curv2 = tk.Label(port_range, text='~')
    label3 = tk.Label(port_range, text='输入线程数：')
    port_entry3 = tk.Entry(port_range, width=12, textvariable=thread_num)

    label2.place(x=70, rely = 0.5, anchor=tk.CENTER)
    port_entry1.place(x=170, rely = 0.5, anchor=tk.CENTER)
    label_curv2.place(x=235, rely = 0.5, anchor=tk.CENTER)
    port_entry2.place(x=300, rely = 0.5, anchor=tk.CENTER)
    label3.place(x=410, rely = 0.5, anchor=tk.CENTER)
    port_entry3.place(x=500, rely = 0.5, anchor=tk.CENTER)

    inputs = {"ip_start": ip_start, "ip_end": ip_end, "port_start": port_start, "port_end": port_end, "thread_num": thread_num}
    button = tk.Button(button_area, height=1, width=7, text="开始扫描", fg='blue', command= lambda:start_scan_thread(inputs, texts))
    button.place(relx=300, rely=0.5, anchor=tk.CENTER)

    button1 = tk.Button(button_area, height=1, width=7, text="停止扫描", fg='blue', command= stop_scan)
    button1.place(x=150, rely=0.5, anchor=tk.CENTER)

    texts = tk.Text(show_area, height=400, width=600)
    scroll = tk.Scrollbar(show_area)
    scroll.pack(side=tk.RIGHT, fill=tk.Y)
    texts.pack(side=tk.LEFT, fill=tk.Y)
    scroll.config(command=texts.yview)
    texts.config(yscrollcommand=scroll.set)

    button2 = tk.Button(button_area, height=1, width=7, text="清空数据", fg='blue', command= lambda: clear_data(texts))
    button2.place(x=450, rely=0.5, anchor=tk.CENTER)

    inputs = {"ip_start": ip_start, "ip_end": ip_end, "port_start": port_start, "port_end": port_end, "thread_num": thread_num}
    button = tk.Button(button_area, height=1, width=7, text="开始扫描", fg='blue', command= lambda:start_scan_thread(inputs, texts))
    button.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
    window.mainloop()


def scan_thread(urls):
    s = socket.socket()
    texts=urls[0]

    for i in range(1, len(urls), 1):
        host, port = urls[i]
        try:
            s.connect(urls[i])
            texts.insert(tk.END, host+': '+str(port)+'...open\n')
        except socket.error:
            texts.insert(tk.END, host+': '+str(port)+'...closed\n')
    s.close()

def start_scan_thread(inputs, texts):
    global scanner
    if scanner is not None and scanner.stopped() is False:
        texts.insert(tk.END, 'scanner is busy. Try again latter!\n')
        return
    scanner = scanthread(inputs, texts)
    scanner.start()
    # threading.Thread(target=start_scan, args=(inputs, texts,)).start()

def stop_scan():
    global scanner
    print(scanner.stopped())
    scanner.stop()
    print(scanner.stopped())

def clear_data(texts):
    texts.delete('0.0', tk.END)


def check_inputs(inputs, texts):
    pattern_p = '^[1-9]{1}[0-9]*$'
    pattern_i = '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$'

    ip_start = inputs['ip_start'].get()
    ip_end = inputs['ip_end'].get()
    port_start = inputs['port_start'].get()
    port_end = inputs['port_end'].get()
    thread_num = inputs['thread_num'].get()

    if re.match(pattern_p, port_start) is False or  re.match(pattern_p, port_end) is False or int(port_end)<=int(port_start) or int(port_end)>=65535:
        texts.insert(tk.END, 'invalid port range!\n')
        return False
    if re.match(pattern_p, thread_num) is False or int(thread_num)>1000:
        texts.insert(tk.END, 'invalid thread num (0~50)!\n')
        return False
    if re.match(pattern_i, ip_start) is False or re.match(pattern_i, ip_end) is False:
        texts.insert(tk.END, 'invalid ip address!\n')
        return False
    
    ip_s = ip_start.split('.')
    ip_e = ip_end.split('.')

    for i in range(4):
        if int(ip_s[i])>255 or int(ip_e[i])>255:
            texts.insert(tk.END, 'invalid ip address!\n')
            return False
    
    if addr2dec(ip_start)>addr2dec(ip_end):
        texts.insert(tk.END, 'invalid ip range!\n')
        return False
    return True


if __name__ == "__main__":
    create_window()