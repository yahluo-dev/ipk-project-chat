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

debug = False

msg_maxlen = 2048

def assert_equal(a, b):
    assert a == b, '%s != %s' % (a, b)

def assert_nequal(a, b):
    assert a != b, '%s == %s' % (a, b)

def assert_in(subs, string):
    assert subs in string, '"%s" not in "%s"' % (subs, string)

def test_case(func):
    def wrapper(*args, **kwargs):
        func_name = func.__name__
        print('=' + func_name + '=' * (40-len(func_name)))
        try:
            func(*args, **kwargs)
            print('[OK]')
        except AssertionError as e:
            print(f'[FAILED]')
            raise
        return
    return wrapper

class TestServer:
    def __init__(self, port=4567, proto='udp', host='127.0.0.1'):
        print("Starting server...")
        self.proto = proto
        if (proto == 'udp'):
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind((host, port))
            self.sock.settimeout(1)
        elif (proto == 'tcp'):
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.bind((host, port))
            self.sock.settimeout(1)
            self.sock.listen()
        else:
            assert(False)
    def recvmsg(self):
        if (self.proto == 'udp'):
            fds = array.array("i")
            try:
                msg, ancdata, flags, addr = self.sock.recvmsg(msg_maxlen, 
                                                            socket.CMSG_LEN(10 * fds.itemsize))
            except TimeoutError:
                return b''
            self.client_port = addr[1]
            return msg
        else:
            try:
                data = self.conn.recv(1024)
                return data
            except TimeoutError:
                return b''

    def close_connection(self):
        if (self.sock):
            self.sock.close()

    def accept(self):
        if (debug):
            print("Server: accepting")
        self.conn, _ = self.sock.accept()

    def sendmsg(self, msg: bytes):
        if (debug):
            print("Server: sending")
        if (self.proto == 'udp'):
            self.sock.sendto(msg, ('127.0.0.1', self.client_port))
        else:
            self.conn.sendall(msg)

    def reply_auth_ok(self):
        self.sendmsg('\x01\x00\x00\x01\x00\x00COOL\x00')

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
                if (not line):
                    continue
                if (debug):
                    print("STDOUT: " + line.strip(), file=sys.stderr)
                self.stdout_queue.put(line)
            #process.stdout.close()
        except ValueError:
            return

    def _read_stderr(self):
        try:
            for line in iter(self.process.stderr.readline, b''):
                if (not line):
                    continue
                if (debug):
                    print("STDERR: " + line.strip(), file=sys.stderr)
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

        self.process = subprocess.Popen(
                [self.exe_path] + self.args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=0
                )
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
        sleep(0.2)
        self.process.stdin.close()
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

    def try_auth(self):
        self.write_stdin('/auth user secret display')

@test_case
def no_args():
    """ Returns an error when no arguments supplied """
    args = []
    client = ClientSUT(exe_path, args)
    client.start()
    client.terminate()
    assert_nequal(0, client.return_code)


@test_case
def help_no_error():
    """ Returns no error with -h argument """
    args = ['-h']
    client = ClientSUT(exe_path, args)
    client.start()
    client.terminate()
    assert_equal(0, client.return_code)


@test_case
def udp_auth_ok():
    """ Shows success on successful authentication """
    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.try_auth()

    sleep(0.2)

    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())
    
    sleep(0.2)

    stderr = client.get_stderr()
    assert_in('Success: COOL', stderr)
    client.terminate()


@test_case
def udp_auth_nok():
    """ Shows failure on unsuccessful authentication """

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.try_auth()

    sleep(0.2)


    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x00\x00\x00NOTCOOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())
    
    sleep(0.2)

    stderr = client.get_stderr()
    assert_in('Failure: NOTCOOL', stderr)
    client.terminate()


@test_case
def udp_auth_ok_bye_on_ctrlc():
    """ Sends a BYE when SIGINT received in OPEN state """

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    client.try_auth()

    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())
    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())

    client.send_signal(2)
    sleep(0.3)

    assert_equal(b'\xff\x00\x01', server.recvmsg())

    server.sendmsg("\x00\x00\x01".encode())

    client.terminate()
    assert_equal(0, client.return_code)


@test_case
def udp_auth_ok_bye_on_ctrld():
    """ Sends a BYE when stdin closed in OPEN state """
    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())
    client.start()

    # Authenticate
    client.try_auth()
    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())

    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())

    # Close stdin
    client.close_stdin()
    sleep(0.3)

    # Expect a BYE sent
    assert_equal(b'\xff\x00\x01', server.recvmsg())
    server.sendmsg("\x00\x00\x01".encode())
    client.terminate()
    assert_equal(0, client.return_code)


@test_case
def udp_auth_ok_sendmsg():
    """ Sends a message to the server when user requests provided the connection is open. """
    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())

    client.start()

    client.try_auth()
    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())

    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x01\x00\x00COOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())

    client.write_stdin('Hello')

    assert_equal(b'\x04\x00\x01display\x00Hello\x00', server.recvmsg())
    server.sendmsg(b'\x00\x00\x01')

    sleep(0.3)
    client.send_signal(2)
    sleep(0.3)

    assert_equal(b'\xff\x00\x02', server.recvmsg())

    server.sendmsg(b'\x00\x00\x02')

    sleep(0.3)

    try:
        client.terminate()
    except TimeoutError:
        print("Client termination timeout")
    assert_equal(0, client.return_code)

@test_case
def udp_auth_nok_sendmsg():
    """ Shows an error when user wants to send a message and the connection is not open. """

    server = TestServer()
    client = ClientSUT(exe_path, '-t udp -s 127.0.0.1 -p 4567'.split())

    client.start()

    client.try_auth()
    assert_equal(b'\x02\x00\x00user\x00display\x00secret\x00', server.recvmsg())

    server.sendmsg("\x00\x00\x00".encode())
    server.sendmsg("\x01\x00\x00\x00\x00\x00NOTCOOL\x00".encode())
    assert_equal(b'\x00\x00\x00', server.recvmsg())

    client.write_stdin('Hello')

    assert_equal(b'', server.recvmsg()) # shouldn't get anything

    stderr = client.get_stderr()
    assert_in('ERR:', stderr)

    sleep(0.3)
    client.send_signal(2)
    sleep(0.3)

    assert_equal(b'', server.recvmsg()) # we're in state OPEN, no bye

    sleep(0.3)

    try: # FIXME
        client.terminate()
    except TimeoutError:
        print("Client termination timeout")
    assert_equal(0, client.return_code)


@test_case
def tcp_auth_ok():
    """ Shows a success message on successful tcp authentication. """
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())

        client.start()
        server.accept()

        client.try_auth()
        sleep(0.2)
        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY OK IS COOL\r\n')
        server.sendmsg(b'MSG FROM Server IS display joined general.\r\n')

        sleep(0.4)

        stderr = client.get_stderr()
        assert_in('Success: COOL', stderr)

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()


@test_case
def tcp_auth_nok():
    """ Shows a failure message on unsuccessful tcp authentication. """
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())

        client.start()

        server.accept()

        client.try_auth()
        sleep(0.2)
        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY NOK IS NOTCOOL\r\n')

        sleep(0.2)

        stderr = client.get_stderr()
        assert_in('Failure: NOTCOOL', stderr)

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()

@test_case
def tcp_auth_ok_sendmsg():
    """ 
    Sends a message upon user request
    provided the connection is established.
    """
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())
        client.start()
        server.accept()

        client.try_auth()

        sleep(0.2)

        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY OK IS COOL\r\n')
        server.sendmsg(b'MSG FROM Server IS display joined general.\r\n')

        sleep(0.2)

        stderr = client.get_stderr()
        assert_in('Success: COOL', stderr)

        client.write_stdin("Hello")

        assert_equal(b'MSG FROM display IS Hello\r\n', server.recvmsg())

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()

@test_case
def tcp_auth_nok_sendmsg():
    """ 
    Shows an error upon user request of sending a message
    provided the connection is not established.
    """
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())

        client.start()

        server.accept()

        client.try_auth()
        sleep(0.2)
        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY NOK IS NOTCOOL\r\n')

        sleep(0.2)

        stderr = client.get_stderr()
        assert_in('Failure: NOTCOOL', stderr)

        client.write_stdin("Hello")

        sleep(0.1)

        stderr = client.get_stderr()
        assert_in('ERR:', stderr)

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()


@test_case
def tcp_auth_ok_bye_on_ctrl_c():
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())
        client.start()
        server.accept()

        client.try_auth()

        sleep(0.2)

        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY OK IS COOL\r\n')
        server.sendmsg(b'MSG FROM Server IS display joined general.\r\n')

        sleep(0.2)

        stderr = client.get_stderr()
        assert_in('Success: COOL', stderr)

        client.send_signal(2)

        sleep(0.3)

        assert_equal(b'BYE\r\n', server.recvmsg())

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()


@test_case
def tcp_auth_ok_bye_on_ctrl_d():
    try:
        server = TestServer(proto='tcp')
        client = ClientSUT(exe_path, '-t tcp -s 127.0.0.1 -p 4567'.split())
        client.start()
        server.accept()

        client.try_auth()

        sleep(0.2)

        assert_equal(b'AUTH user AS display USING secret\r\n', server.recvmsg())
        server.sendmsg(b'REPLY OK IS COOL\r\n')
        server.sendmsg(b'MSG FROM Server IS display joined general.\r\n')

        sleep(0.2)

        stderr = client.get_stderr()
        assert_in('Success: COOL', stderr)

        client.close_stdin()

        sleep(0.3)

        assert_equal(b'BYE\r\n', server.recvmsg())

        server.close_connection()

    except OSError:
        print(e)
        exit(1)
    except Exception as e:
        server.close_connection()
        raise
    finally:
        client.terminate()

def test_main():
    if (len(sys.argv) < 2):
        exit(1)
    global exe_path
    exe_path = sys.argv[1]

    general_tests = True
    udp_tests = True
    tcp_tests = True

    if (general_tests == True):
        no_args()
        help_no_error()
    if (udp_tests == True):
        udp_auth_ok()
        udp_auth_ok_bye_on_ctrlc()
        udp_auth_ok_sendmsg()
        udp_auth_nok_sendmsg()
    if (tcp_tests == True):
        tcp_auth_ok()
        tcp_auth_nok()
        tcp_auth_ok_sendmsg()
        tcp_auth_nok_sendmsg()
        tcp_auth_ok_bye_on_ctrl_c()
        tcp_auth_ok_bye_on_ctrl_d()

if __name__ == "__main__":
    test_main()
