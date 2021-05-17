#!/usr/bin/python3

import serial
import struct
import time
from timeit import default_timer

def _make_serial_connection():
    port = serial.Serial('/dev/ttyACM0', 9600)
    port.flush()
    time.sleep(2)  # Time required since arduino will reset
    # Wipe state of the buffer from Jetson's side
    return port

def _check_for_awake(port, timeout=2):
    found = False
    start = default_timer()
    while((found == False) and ((default_timer() - start) < timeout)):
        line = port.readline()
        if (line == b'AWAKE\r\n'):
            found = True

    return found

def _format_packet(deflections):
    packet = struct.pack('<BB',  deflections[0], deflections[1])
    packet = b'[' + packet + b']'
    return packet

def _send_packet_to_arduino(port, pkt):
    port.flush()
    port.write(pkt)
    # time.sleep(0.5)  # Buffer time to write to Serial

def _wait_for_acknowledge(port, seconds, expected):
    exp = b"{},{}".format(expected[0],expected[1])
    # expected_bytes = bytes(exp, encoding='utf-8')
    start = default_timer()
    received = False
    while ((default_timer() - start) < seconds):
        if (port.in_waiting):
            new_line = port.readline()
            #print("Got: \t\t{}\r\n".format(new_line.strip()))
            if (new_line.startswith(exp)):
                # print("Got ack")
                received = True
                break

    if received == False:
        print("FAILED TO RECEIVE ACKNOWLEDGEMENT STRING FROM ARDUINO\r\n")

    return

