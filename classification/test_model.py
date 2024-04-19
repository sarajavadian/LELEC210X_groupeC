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
import warnings
warnings.filterwarnings("ignore")

from classification.utils.plots import plot_specgram

PRINT_PREFIX = "DF:HEX:"
FREQ_SAMPLING = 10200
MELVEC_LENGTH = 20
N_MELVECS = 20

MEMORY_SIZE = 5
THRESH_SOUND = 0
THRESH_PROBA = 0
WEIGHT_MEMO = 0.5
classnames = ["birds", "chainsaw","fire","handsaw","helicopter"]


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


def add_in_memory(m, idx, vp) :
    if idx < MEMORY_SIZE :
        m[idx] = vp
        return idx+1
    else:
        for i in range (MEMORY_SIZE-1):
            m[i] = m[i+1]
        m[-1] = vp
        return idx
    
def Naive(memo) :
    idx = np.where(memo == np.max(memo))
    return classnames[idx[1][0]], memo[idx[0][0]][idx[1][0]]

def Majority_voting(memo) :
    counter_max = np.zeros(len(classnames))
    for i in range(len(memo)):
        idx = np.where(memo[i] == np.max(memo[i]))
        counter_max[idx[0][0]] += 1
    idx = np.where(counter_max == np.max(counter_max))
    return classnames[idx[0][0]], counter_max[idx[0][0]] / np.sum(counter_max)
    
def Averaging_FV(memo) :
    av_prob = np.sum(memo, axis=0) / len(classnames)
    idx = np.where(av_prob == np.max(av_prob))
    return classnames[idx[0][0]], av_prob[idx[0][0]]

def ML(memo) :
    logs = np.log(memo)
    loglikelihood = np.sum(logs,axis=0)
    idx = np.where(loglikelihood == np.max(loglikelihood))
    return classnames[idx[0][0]], loglikelihood[idx[0][0]]



if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument("-p", "--port", help="Port for serial communication")
    args = argParser.parse_args()
    print("uart-reader launched...\n")
    
    
    model = "data/models/RFT_with_proba_V2_normalized.pickle"
    with open(model, "rb") as file:
        m = pickle.load(file)
        print("model loaded")
    
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
       
        for melvec in input_stream:
            msg_counter += 1
          
            print(f"MEL Spectrogram #{msg_counter}")
            
            #print("norm :", np.linalg.norm(melvec))
            
            result = m.predict( melvec.reshape((1, MELVEC_LENGTH*N_MELVECS)) )
            prob = m.predict_proba( melvec.reshape((1, MELVEC_LENGTH*N_MELVECS)) )
          
            tot_prob = np.concatenate((WEIGHT_MEMO * memory, prob[0].reshape((1,5))))
            
            melvec_normalized = melvec / np.linalg.norm(melvec)
            result_normalized = m.predict( melvec_normalized.reshape((1, MELVEC_LENGTH*N_MELVECS)) )
            
            
            print(result[0])
            print(result_normalized[0])
            print()
            #print("Result with memory:")
            
            #res, p = Naive(tot_prob)
            #print(res)
            #res, p = Majority_voting(tot_prob)
            #print(res)
            res, p = Averaging_FV(tot_prob)
            #print(res)
            #res, p = ML(tot_prob)
            #print(res)
            
            
            print()
            print()
                
            idx_empty = add_in_memory(memory,idx_empty, prob[0])
            
        