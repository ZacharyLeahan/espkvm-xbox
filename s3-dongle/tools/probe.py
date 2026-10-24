#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Discover the paired S3 and optionally verify neutral packet/replay handling.

Run before the P4 is routing, while the S3 is attached to the Mac. This tool
never sends a pressed control. Pairing material stays in ignored .env.s3-route.
"""
import argparse
import hashlib
import hmac
import json
from pathlib import Path
import secrets
import socket
import struct
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
WIRE = struct.Struct('<4sBB2xIII6s2x20s32s')

def signed(key, kind, sequence, session, receiver, mac, report):
    raw = WIRE.pack(b'X3P1', kind, 0, sequence, session, receiver, mac, report, bytes(32))
    return raw[:48] + hmac.digest(key, raw[:48], 'sha256')

def discover(timeout=20):
    private = json.loads((ROOT / '.env.s3-route').read_text())
    key = bytes.fromhex(private['key'])
    mac = bytes.fromhex(private['mac'].replace(':', ''))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', 3335))
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        sock.settimeout(max(.1, deadline-time.monotonic()))
        raw, addr = sock.recvfrom(1024)
        if len(raw) != WIRE.size or not hmac.compare_digest(raw[48:], hmac.digest(key,raw[:48],'sha256')):
            continue
        magic, kind, flags, seq, session, receiver, peer, report, tag = WIRE.unpack(raw)
        if magic == b'X3P1' and kind == 1 and peer == mac:
            return sock, key, mac, session, addr[0], bool(flags&1)
    raise TimeoutError('Paired S3 not discovered')

def status(ip):
    with urllib.request.urlopen(f'http://{ip}/status',timeout=4) as response:
        return response.read().decode()

def count(text):
    return int(text.split('P4 reports: ',1)[1].split(' |',1)[0])

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test-neutral',action='store_true')
    args=parser.parse_args()
    sock,key,mac,boot,ip,usb=discover()
    print(f'Authenticated S3 at {ip}, USB configured={usb}')
    print(status(ip))
    if not args.test_neutral:return
    session=secrets.randbits(32)
    neutral=bytes([0,20])+bytes(18)
    before=count(status(ip))
    packet=signed(key,2,10,session,boot,mac,neutral)
    sock.sendto(packet,(ip,3334));time.sleep(.1)
    assert count(status(ip))==before+1,'valid neutral report not accepted (is the P4 already routing?)'
    sock.sendto(packet,(ip,3334))  # duplicate
    sock.sendto(signed(key,2,9,session,boot,mac,neutral),(ip,3334))  # old sequence
    sock.sendto(signed(key,2,11,session,boot^1,mac,neutral),(ip,3334))  # old boot
    invalid=bytearray(signed(key,2,12,session,boot,mac,neutral));invalid[-1]^=1
    sock.sendto(invalid,(ip,3334))
    sock.sendto(signed(key,2,13,session,boot,mac,neutral)+b'x',(ip,3334))
    time.sleep(.1)
    assert count(status(ip))==before+1,'invalid/replayed packet accepted'
    time.sleep(.55)
    assert 'routing: waiting' in status(ip),'input lease did not expire'
    print('PASS: valid report accepted; duplicate, reordered, old-boot, bad-MAC and oversized packets rejected; lease expired.')

if __name__=='__main__':main()
