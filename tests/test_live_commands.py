"""Load the built DLL, send real TCP/WebSocket commands, inspect SDK Update output.

Run with Aerofly and other bridge hosts closed. No aircraft simulation is invented.
"""
import argparse
import ctypes as c
import json
import os
import socket
import struct
import subprocess
import time
import threading

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--dll',required=True)
    parser.add_argument('--catalog',required=True)
    args=parser.parse_args()
    for port in (12345,12346,8765):
        with socket.socket() as probe:
            if probe.connect_ex(('127.0.0.1',port))==0:
                raise RuntimeError(f'Port {port} is occupied; close other bridge hosts first')
    manifest=json.loads(subprocess.check_output([args.catalog,'--manifest']))
    os.environ['AEROFLY_BRIDGE_LOG_CONSOLE']='0'
    dll=c.CDLL(os.path.abspath(args.dll))
    prefix='Aerofly_FS_4_External_DLL_'
    init=getattr(dll,prefix+'Init'); init.argtypes=[c.c_void_p]; init.restype=c.c_bool
    update=getattr(dll,prefix+'Update')
    update.argtypes=[c.c_double,c.c_void_p,c.c_uint32,c.c_uint32,c.c_void_p,c.POINTER(c.c_uint32),c.POINTER(c.c_uint32),c.c_uint32]
    update.restype=None
    shutdown=getattr(dll,prefix+'Shutdown'); shutdown.restype=None
    output=c.create_string_buffer(4*1024*1024)
    def pump():
        size=c.c_uint32(); count=c.c_uint32()
        update(.02,None,0,0,output,c.byref(size),c.byref(count),len(output))
        data=output.raw[:size.value]; offset=0; messages=[]
        for _ in range(count.value):
            length=struct.unpack_from('<H',data,offset+2)[0]
            assert length>=64 and offset+length<=len(data),'Invalid SDK output size'
            messages.append((struct.unpack_from('<Q',data,offset+16)[0],data[offset+25],
                struct.unpack_from('<Q',data,offset+56)[0],data[offset+64:offset+length]))
            offset+=length
        return messages
    def verify(request,send):
        send(json.dumps({k:request[k] for k in ('variable','value','flag')}).encode())
        deadline=time.monotonic()+3
        while time.monotonic()<deadline:
            for mid,typ,flags,payload in pump():
                if mid!=request['id'] or not flags & (1<<26): continue
                assert typ==request['type'],request
                assert flags & request['flags']==request['flags'],request
                value=request['value']
                if isinstance(value,str): assert payload.decode('utf-16-le').rstrip('\0')==value,request
                else:
                    expected=value if isinstance(value,list) else [value]
                    assert list(struct.unpack('<'+'d'*len(expected),payload))==expected,request
                return
            time.sleep(.01)
        raise AssertionError(f'No SDK command received: {request}')
    assert init(None),'DLL initialization failed'
    try:
        def tcp(packet):
            with socket.create_connection(('127.0.0.1',12346),timeout=2) as client:
                client.sendall(packet+b'\n'); client.shutdown(socket.SHUT_WR)
        for request in manifest: verify(request,tcp)
        for request in ({'variable':'Aircraft.PitchTrim','value':1},
                        {'variable':'Controls.Trim','flag':'Unknown','value':1},
                        {'variable':'Controls.Trim','value':None}):
            tcp(json.dumps(request).encode())
            deadline=time.monotonic()+.1
            while time.monotonic()<deadline:
                assert not any(flags & (1<<26) for _,_,flags,_ in pump()),'Invalid command was forwarded'
                time.sleep(.01)
        with socket.create_connection(('127.0.0.1',8765),timeout=2) as ws:
            ws.sendall(b'GET / HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n')
            assert b'101 Switching Protocols' in ws.recv(4096),'WebSocket upgrade failed'
            # Real clients consume telemetry while sending commands. Prevent our
            # test client's unread receive buffer from stalling the server.
            draining=threading.Event()
            def drain():
                while not draining.is_set():
                    try:
                        if not ws.recv(65536): return
                    except socket.timeout: continue
                    except OSError: return
            reader=threading.Thread(target=drain,daemon=True)
            reader.start()
            def websocket(packet):
                mask=b'F2GT'
                header=bytes([0x81,0x80|len(packet)]) if len(packet)<126 else b'\x81\xfe'+struct.pack('>H',len(packet))
                ws.sendall(header+mask+bytes(b^mask[i%4] for i,b in enumerate(packet)))
            for request in manifest:
                verify(request,websocket)
            draining.set()
        print(f'PASS: {len(manifest)} writable variants through TCP and {len(manifest)} through WebSocket, actual DLL SDK output verified.')
    finally: shutdown()

if __name__=='__main__': main()
