import matplotlib.pyplot as plt
import numpy as np
import scipy

R = 99.9 #value of the used resistance

file = "8MHz,SRAM1_Deactivated_99.9Ohm_5.csv"
f = open(file, "r")
text = f.readlines()
nbr_samples = int(text[3].split(",")[1]) # extract the number of samples
print(nbr_samples)
text = text[8:] # discard irrelevant information

X = np.zeros(nbr_samples)
Y = np.zeros(nbr_samples)

for i, line in enumerate(text):
    splitted = line.split(",")
    X[i] = float(splitted[1])
    Y[i] = float(splitted[2])
X = X - X[0] # -X[0] to remove the time offset
Y = (3.3-Y)* Y/R # function to apply to Y
Y *= 1000 # in mW instead of W
print("min : " + str(np.min(Y)))

a, b = 0, nbr_samples # boundaries between which a unique cycle appears
X, Y = X[a:b], Y[a:b]
avg_pwr = np.mean(Y)
print("Average power over one cycle : {0} mW".format(avg_pwr))
print("Energy consumption over one cycle : {0} mW*s".format(scipy.integrate.simps(Y, X)))
print("Energy consumption over one cycle : {0} µW*h".format(scipy.integrate.simps(Y, X)*1000/3600))

plt.plot(X, Y) 
plt.ylim((0, np.max(Y)*1.1))
plt.ylabel("Power consumed [mW]")
plt.xlabel("Time [s]")
plt.title("Evolution of the power consumed over time")
plt.show()
