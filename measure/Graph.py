import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import scipy as sc

frq = [0.1, 4, 8, 16, 32]
frq_extended = [0.125, 0.25, 0.5, 1] + frq
frq_continuous = np.arange(2, 32, 0.1)
Pwr_vs_frq = [11.82, 8.19, 6.03, 4.60]
Pwr_vs_frq =  Pwr_vs_frq[::-1]
SDC = [1, 44.0, 59.5, 70.2, 77.8]

V = [3.3, 3, 2.7, 2.4, 2.1, 1.8]
Pwr_vs_V = [3.87, 3.42, 3.10, 2.68, 2.43, 2.11]

coeff = sc.optimize.curve_fit(lambda t,a,b,c: (-a/(t+c)) + b, frq, SDC)
pwr_extended = (-coeff[0][0]/(frq_continuous+coeff[0][2])) + coeff[0][1]

# coeff2 = sc.optimize.curve_fit(lambda t,a,b,c: a*np.log(b*t+c), frq, SDC)
# pwr_extended2 = coeff2[0][0] * np.log(coeff2[0][1] * frq_continuous + coeff2[0][2])
# print(pwr_extended2)

print("here : ")
print(coeff[0][:])

# plt.subplot(311)
# plt.scatter(frq, Pwr_vs_frq, color='blue', label='Power vs frequency')
# plt.plot(frq_extended, pwr_extended, color='purple', label='Linear interpolation', linestyle="-.")
# plt.xlabel("System clock frequency (in MHz)")
# plt.ylabel("Mean power consumption (mW)")
# plt.xticks(np.arange(0, 34, 4))
# plt.legend()

# plt.subplot(312)
plt.scatter(frq[1:], SDC[1:], color='blue', label='Measure of the duty cycle')
plt.plot(frq_continuous, pwr_extended, color='purple', label='Fitted interpolation', linestyle="-.")
# plt.plot(frq_continuous, pwr_extended2, color='orange', label='Linear interpolation', linestyle="--")
plt.xlabel("Frequency (in MHz)")
plt.ylabel("Sensing duty cycle (in %)")
plt.xlim((0, 33))
plt.xticks(np.arange(2, 34, 2))
plt.legend()

# plt.subplot(313)
# plt.scatter(V, Pwr_vs_V, color='blue')
# plt.plot(V, pwr_extended, color='purple', label='Linear interpolation', linestyle="-.")
# plt.xlabel("Supply voltage (in V)")
# plt.ylabel("Mean power consumption (in mW)")

plt.savefig("TAG_SDC.pdf")
plt.show()