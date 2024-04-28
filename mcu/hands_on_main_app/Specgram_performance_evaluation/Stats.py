import numpy as np
import matplotlib.pyplot as plt

f = open("Assistant_implementation.txt", "r")
g = open("Our_implementation.txt", "r")

textf = f.readlines()
textg = g.readlines()

f.close()
g.close()

for line in textf:
    