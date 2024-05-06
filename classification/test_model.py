"""
uart-reader.py
ELEC PROJECT - 210x
"""
import argparse

import matplotlib.pyplot as plt
import numpy as np
import time
import serial
from serial.tools import list_ports
import pickle
import warnings
warnings.filterwarnings("ignore")
from classification.utils.plots import plot_specgram



PRINT_PREFIX = "DF:HEX:"
FREQ_SAMPLING = 10200
MELVEC_LENGTH = 20
N_MELVECS = 20

MEMORY_SIZE = 5
WEIGHT_MEMO = 0.5
classnames = ["birds", "chainsaw","fire","handsaw","helicopter"]


dt = np.dtype(np.uint16).newbyteorder("<")

def parse_buffer(line):
    line = line.strip()
    if line.startswith(PRINT_PREFIX):
        return bytes.fromhex(line[(len(PRINT_PREFIX) + 16) : 1623])  # modified to not keep header and tag (only payload)
        #return bytes.fromhex(line[len(PRINT_PREFIX) :])
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






def add_in_memory(m, idx, vp) :
    if idx < MEMORY_SIZE :
        m[idx] = vp
        return idx+1
    else:
        for i in range (MEMORY_SIZE-1):
            m[i] = m[i+1]
        m[-1] = vp
        return idx
    

def Averaging_FV(memo) :
    av_prob = np.sum(memo, axis=0) / len(classnames)
    idx = np.where(av_prob == np.max(av_prob))
    return classnames[idx[0][0]], av_prob[idx[0][0]]





if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument("-p", "--port", help="Port for serial communication")
    args = argParser.parse_args()
    print("uart-reader launched...\n")
    
    # init des modèles
    model1 = "data/models/rft_final_dataset.pickle"
    with open(model1, "rb") as file:
        m1 = pickle.load(file)
        print("model 1 loaded")
    
    # init memoire
    memory = np.zeros((MEMORY_SIZE,len(classnames)))
    idx_empty = 0
    
    
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
        
        start = time.time()
        for melvec in input_stream:
            end = time.time()
            
            msg_counter += 1
            print(f"MEL Spectrogram #{msg_counter}")
            
            if end-start > 3.0 :
                print("- Memory reset -")
                memory = np.zeros((MEMORY_SIZE,len(classnames)))
                idx_empty = 0
    
            melvec_normalized = melvec / np.linalg.norm(melvec)
            melvec_normalized = melvec_normalized.reshape((1, MELVEC_LENGTH*N_MELVECS))
            
            current_prob1 = m1.predict_proba(melvec_normalized)
            tot_prob = np.concatenate( ( WEIGHT_MEMO * memory, current_prob1[0].reshape((1,5)) ) )
            prediction, proba = Averaging_FV(tot_prob)
            idx_empty = add_in_memory(memory, idx_empty, current_prob1[0])
            
            print(prediction)
            print()
            
            start = time.time()
            
            
