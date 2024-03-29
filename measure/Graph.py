import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import scipy as sc

frq = [2, 4, 8, 16, 32]
frq_extended = [0.125, 0.25, 0.5, 1] + frq
Pwr_vs_frq = [11.82, 8.19, 6.03, 4.60, 3.87]
Pwr_vs_frq =  Pwr_vs_frq[::-1]
SDC = [32.2, 44.0, 59.5, 70.2, 77.8]

V = [3.3, 3, 2.7, 2.4, 2.1, 1.8]
Pwr_vs_V = [3.87, 3.42, 3.10, 2.68, 2.43, 2.11]

coeff = sc.optimize.curve_fit(lambda t,a,b,c: a*t**2 + b*t + c, frq, Pwr_vs_frq)
pwr_extended = coeff[0][0] * np.power(frq_extended,2) + coeff[0][1] * np.power(frq_extended,1) + coeff[0][2]
print(pwr_extended)

plt.subplot(311)
plt.plot(frq, Pwr_vs_frq, color='blue', label='Power vs frequency (in mW)')
plt.plot(frq_extended, pwr_extended, color='purple', label='Extrapolations', linestyle="-.")
plt.gca().invert_xaxis()
plt.xlabel("Frequency (in Mhz)")
plt.ylabel("Mean power consumed (mW)")
plt.xscale("log", base=2)
plt.legend()

plt.subplot(312)
plt.plot(frq, SDC, color='red', label='Sensing duty cycle vs frequency (%)')
plt.gca().invert_xaxis()
plt.xlabel("Frequency (in Mhz)")
plt.ylabel("Sensing duty cycle (in %)")
plt.xscale("log", base=2)
plt.legend()

plt.subplot(313)
plt.plot(V, Pwr_vs_V, color='green')
plt.gca().invert_xaxis()
plt.xlabel("Supply voltage (in V)")
plt.ylabel("Mean power consumption (in mW)")

plt.savefig("Meeting2.jpg")
plt.show()