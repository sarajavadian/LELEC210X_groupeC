"""
uart-reader.py
ELEC PROJECT - 210x
"""
import argparse

import matplotlib.pyplot as plt
import numpy as np
import serial
from serial.tools import list_ports
import pickle

from classification.utils.plots import plot_specgram

PRINT_PREFIX = "DF:HEX:"
FREQ_SAMPLING = 10200
MELVEC_LENGTH = 20
N_MELVECS = 20

dt = np.dtype(np.uint16).newbyteorder("<")


def parse_buffer(line):
    line = line.strip()
    if line.startswith(PRINT_PREFIX):
        return bytes.fromhex(line[(len(PRINT_PREFIX) + 16) : 1623])  # modified to not keep header and tag (only payload)
    else:
        #print(line)
        return None


def reader(port=None):
    ser = serial.Serial(port=port, baudrate=115200)
    while True:
        line = ""
        while not line.endswith("\n"):
            line += ser.read_until(b"\n", size=2 * N_MELVECS * MELVEC_LENGTH).decode(
                "ascii"
            )
             #print(line)
        
        line = line.strip()
    
        buffer = parse_buffer(line)
        if buffer is not None:
            buffer_array = np.frombuffer(buffer, dtype=dt)

            yield buffer_array


if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument("-p", "--port", help="Port for serial communication")
    args = argParser.parse_args()
    print("uart-reader launched...\n")
    
    
    model = "data/models/Our_RFT_V2.pickle"
    with open(model, "rb") as file:
        m = pickle.load(file)
        print("model loaded")
    

    if args.port is None:
        print(
            "No port specified, here is a list of serial communication port available"
        )
        print("================")
        port = list(list_ports.comports())
        for p in port:
            print(p.device)
        print("================")
        print("Launch this script with [-p PORT_REF] to access the communication port")

    else:
        input_stream = reader(port=args.port)
        msg_counter = 0
       
        for melvec in input_stream:
            msg_counter += 1
          
            print(f"MEL Spectrogram #{msg_counter}")
            
            melvec = melvec.reshape((1, MELVEC_LENGTH*N_MELVECS))
            result = m.predict(melvec)
            print(result)
            print()
                
            
        