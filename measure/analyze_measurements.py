import matplotlib.pyplot as plt
import numpy as np
import scipy

file = "TAG_shortened/4MHz_Tag-2.1-14.5.csv"

R = float(file.split("-")[2][:-4]) #value of the used resistance
V_sup = float(file.split("-")[1]) #value of the supplied voltage
#V_sup -= 0.037 #offset of the ADALM

f = open(file, "r")
text = f.readlines()
f.close()
nbr_samples = int(text[3].split(",")[1]) # extract the number of samples
text = text[8:] # discard irrelevant information

X = np.zeros(nbr_samples)
Y = np.zeros(nbr_samples)

for i, line in enumerate(text):
    splitted = line.split(",")
    X[i] = float(splitted[1])
    Y[i] = float(splitted[2])
X = X - X[0] # -X[0] to remove the time offset
I = Y/R
Y = (V_sup-Y)* I # function to apply to Y
Y *= 1000 # in mW instead of W
print("min : " + str(np.min(Y)) + " mW")
print("min : " + str(1e3 * np.min(I)) + " mA")

a, b = 0, nbr_samples # boundaries between which a unique cycle appears
X, Y = X[a:b], Y[a:b]
avg_pwr = np.mean(Y)
print("Average power : {0} mW".format(avg_pwr))
print("Average current : {0} mA".format(1e3*np.mean(I)))
#print("Energy consumption : {0} mW*s".format(scipy.integrate.simps(Y, X)))
#print("Energy consumption : {0} µW*h".format(scipy.integrate.simps(Y, X)*1000/3600))

plt.plot(X, Y) 
plt.ylim((0, np.max(Y)*1.1))
plt.ylabel("Power consumed [mW]")
plt.xlabel("Time [s]")
plt.title(file.split("-")[0].split("/")[-1].replace("_", " "))
plt.savefig("Graph_power_consumption.jpg")
plt.show()
