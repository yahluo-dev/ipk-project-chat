#!/bin/python3
import subprocess
import sys
import socket
import array
import queue
import threading
from time import sleep

global exe_path
exe_path=""

debug = True

msg_maxlen = 2048

class TestServer:
    def __init__(self, port=4567):
        print("Starting server")
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('127.0.0.1', port))
        self.sock.settimeout(1)
    def recvmsg(self):
        fds = array.array("i")
        try:
            msg, ancdata, flags, addr = self.sock.recvmsg(msg_maxlen, 
                                                        socket.CMSG_LEN(10 * fds.itemsize))
        except TimeoutError:
            return ''
        print(addr)
        self.client_port = addr[1]
        return msg

    def sendmsg(self, msg: bytes):
        self.sock.sendto(msg, ('127.0.0.1', self.client_port))
        pass

class ClientSUT:
    def __init__(self, exe_path, args):
        self.exe_path = exe_path
        self.process = None
        self.stdout_queue = queue.Queue()
        self.stderr_queue = queue.Queue()
        self.args = args
        self.return_code = None

    def _read_stdout(self):
        try:
            for line in iter(self.process.stdout.readline, b''):
                if (debug):
                    print("STDOUT: " + line, file=sys.stderr)
                self.stdout_queue.put(line)
            #process.stdout.close()
        except ValueError:
            return

    def _read_stderr(self):
        try:
            for line in iter(self.process.stderr.readline, b''):
                if (debug):
                    print("STDERR: " + line, file=sys.stderr)
                self.stderr_queue.put(line)
            #process.stdout.close()
        except ValueError:
            return

    def write_stdin(self, data):
        self.process.stdin.write(data + "\n")
        self.process.stdin.flush()

    def start(self):
        self.stdout_queue = queue.Queue()
        self.stderr_queue = queue.Queue()

        print([self.exe_path] + self.args)
        self.process = subprocess.Popen(
                [self.exe_path] + self.args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
                )
        print(self.process)
        self.stdout_thread = threading.Thread(target=self._read_stdout)
        self.stderr_thread = threading.Thread(target=self._read_stderr)
        self.stdout_thread.start()
        self.stderr_thread.start()

    def get_stdout(self):
        data = []
        while not self.stdout_queue.empty():
            data.append(self.stdout_queue.get())
        return ''.join(data)


    def get_stderr(self):
        data = []
        while not self.stderr_queue.empty():
            data.append(self.stderr_queue.get())
        return ''.join(data)

    def terminate(self):
        self.process.stdin.close()
        self.process.wait(0.9)
        self.process.stdout.close()
        self.process.stderr.close()
        self.stdout_thread.join()
        self.stderr_thread.join()

        self.process.terminate()
        self.return_code = self.process.returncode

    def send_signal(self, signal):
        self.process.send_signal(signal)

    def close_stdin(self):
        self.process.stdin.close()


def no_args():
    print("no_args")

    args = []
    client = ClientSUT(exe_path, args)
    client.start()
    client.terminate()
    assert(0 != client.return_code)


def help_no_error():
    print("=========help_no_error=============")

    args = ['-h']
    client = ClientSUT(exe_path, args)
    client.start()
    client.terminate()
    assert(0 == client.return_code)


def udp_auth_ok():
    print("=========udp_auth_ok=============")
    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.write_stdin('/auth user secret display')

    sleep(0.2)

    print(server.recvmsg())

    assert(b'\x02\x00\x00user\x00display\x00secret\x00' == server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    assert(b'\x00\x00\x00' == server.recvmsg())
    
    sleep(0.2)

    stderr = client.get_stderr()
    assert('Success: COOL' in stderr)
    client.terminate()


def udp_auth_nok():
    print('udp_auth_nok')

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.write_stdin('/auth user secret display')

    sleep(0.2)

    print(server.recvmsg())

    assert(b'\x02\x00\x00user\x00display\x00secret\x00' == server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x00\x00\x00NOTCOOL\x00".encode())
    assert(b'\x00\x00\x00' == server.recvmsg())
    
    sleep(0.2)

    stderr = client.get_stderr()
    assert('Failure: NOTCOOL' in stderr)
    client.terminate()


def udp_auth_ok_bye_on_ctrlc():
    print("====udp_auth_ok_bye_on_ctrlc========")

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.write_stdin('/auth user secret display')

    print(server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    print(server.recvmsg())

    client.send_signal(2)
    sleep(0.3)

    bye_msg = server.recvmsg()
    print(bye_msg)
    assert(b'\xff\x00\x01' == bye_msg)

    server.sendmsg("\x00\x00\x01".encode())

    client.terminate()
    assert(0 == client.return_code)

def udp_auth_ok_bye_on_ctrld():
    print("====udp_auth_ok_bye_on_ctrld========")

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.write_stdin('/auth user secret display')

    print(server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    print(server.recvmsg())

    client.close_stdin()
    sleep(0.3)

    bye_msg = server.recvmsg()
    print(bye_msg)
    assert(b'\xff\x00\x01' == bye_msg)

    server.sendmsg("\x00\x00\x01".encode())

    client.terminate()
    assert(0 == client.return_code)


def test_main():
    if (len(sys.argv) < 2):
        exit(1)
    global exe_path
    exe_path = sys.argv[1]
    no_args()
    help_no_error()
    udp_auth_ok()
    udp_auth_ok_bye_on_ctrlc()

if __name__ == "__main__":
    test_main()
