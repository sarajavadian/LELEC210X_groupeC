import numpy as np
import matplotlib.pyplot as plt

f = open("Our_implementation.txt", "r")
g = open("Assistant_implementation.txt", "r")

textf = f.readlines()
textg = g.readlines()

f.close()
g.close()

arrayf = []
arrayg = []

for line in textf:
    temp = ""
    for char in line:
        if char == "[" or char == "\n" or char == "\t" or char == ' ':
            if temp != "":
                arrayf.append(int(temp))
                temp = ""
        else:
            if char in "0123456789":
                temp+=char

for line in textg:
    temp = ""
    for char in line:
        if char == "[" or char == "\n" or char == "\t" or char == ' ':
            if temp != "":
                arrayg.append(int(temp))
                temp = ""
        else:
            if char in "0123456789":
                temp+=char

diff = []
percent_error = []
for i in range(len(arrayf)):
    diff.append(np.abs(arrayf[i]-arrayg[i]))
    if arrayf[i]!=0:
        percent_error.append(np.abs((arrayg[i] - arrayf[i])/arrayf[i]))
    else:
        percent_error.append(0)

mean = np.mean(diff)
std = np.std(diff)
print(arrayf)
print(arrayg)
print(diff)
print("Mean error: {0} ;\n Std : {1} ;\n Max error : {2}".format(mean, std, max(diff)))
print("Mean value of arrayf : {0}".format(np.mean(arrayf)))
print(percent_error)
print("mean error in pourcent : {0}".format(np.mean(percent_error)))
