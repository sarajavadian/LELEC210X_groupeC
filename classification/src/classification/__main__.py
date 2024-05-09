import pickle
from .utils.plots import plot_specgram
import matplotlib.pyplot as plt
import time
from pathlib import Path
from typing import Optional
import numpy as np

import click

import requests

import common
from auth import PRINT_PREFIX
from common.env import load_dotenv
from common.logging import logger

from .utils import payload_to_melvecs
MEMORY_SIZE = 5
WEIGHT_MEMO = 0.5
classnames = ["birds", "chainsaw","fire","handsaw","helicopter"]

load_dotenv()

def Averaging_FV(memo) :
    av_prob = np.sum(memo, axis=0) / len(classnames)
    idx = np.where(av_prob == np.max(av_prob))
    return classnames[idx[0][0]], av_prob[idx[0][0]]

def add_in_memory(m, idx, vp) :
    if idx < MEMORY_SIZE :
        m[idx] = vp
        return idx+1
    else:
        for i in range (MEMORY_SIZE-1):
            m[i] = m[i+1]
        m[-1] = vp
        return idx
    

@click.command()
@click.option(
    "-i",
    "--input",
    "_input",
    default="-",
    type=click.File("r"),
    help="Where to read the input stream. Default to '-', a.k.a. stdin.",
)
@click.option(
    "-m",
    "--model",
    default=None,
    type=click.Path(exists=True, dir_okay=False, path_type=Path),
    help="Path to the trained classification model.",
)
@common.click.melvec_length
@common.click.n_melvecs
@common.click.verbosity
def main(
    _input: Optional[click.File],
    model: Optional[Path],
    melvec_length: int,
    n_melvecs: int,
) -> None:
    """
    Extract Mel vectors from payloads and perform classification on them.
    Classify MELVECs contained in payloads (from packets).

    Most likely, you want to pipe this script after running authentification
    on the packets:

        poetry run auth | poetry run classify

    This way, you will directly receive the authentified packets from STDIN
    (standard input, i.e., the terminal).
    """
    if model:
        with open(model, "rb") as file:
            m1 = pickle.load(file)
            
    else:
        m1 = None
        
    # init memoire
    memory = np.zeros((MEMORY_SIZE,len(classnames)))
    idx_empty = 0
    
    start = time.time()
    for payload in _input:
        end = time.time()
        
        if PRINT_PREFIX in payload:
            payload = payload[len(PRINT_PREFIX) :]

            melvec = payload_to_melvecs(payload, melvec_length, n_melvecs)
            # logger.info(f"Parsed payload into Mel vectors: {melvec}")

            if m1:
                #plot, ax = plt.subplots()
                #plot_specgram(melvecs, ax)
                #plt.show()
                #melvecs = melvecs.reshape((1, melvec_length*n_melvecs))
                #result = m.predict(melvecs)
                #print(result)
                
                if end-start > 3.0 :
                    print("- Memory reset -")
                    memory = np.zeros((MEMORY_SIZE,len(classnames)))
                    idx_empty = 0

                melvec_normalized = melvec / np.linalg.norm(melvec)
                melvec_normalized = melvec_normalized.reshape((1, melvec_length*n_melvecs))
                
                current_prob1 = m1.predict_proba(melvec_normalized)
                tot_prob = np.concatenate( ( WEIGHT_MEMO * memory, current_prob1[0].reshape((1,5)) ) )
                prediction, proba = Averaging_FV(tot_prob)
                idx_empty = add_in_memory(memory, idx_empty, current_prob1[0])
                
                print(prediction)
                print()
            
                hostname = "https://lelec210x.sipr.ucl.ac.be"
                #hostname = "http://localhost:5000"
                key = "aqH27o66E8xz-IotBk11ZZo1ix7Vbs5H2pTXlSra"
                guess = prediction
                response = requests.post(f"{hostname}/lelec210x/leaderboard/submit/{key}/{guess}", timeout=1)
                
                start = time.time()

