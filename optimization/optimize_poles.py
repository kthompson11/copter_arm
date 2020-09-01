import numpy
import matplotlib.colors
import matplotlib.pyplot as plt
import scipy.optimize

#kd = 0.5
TimerMax = 640  # max value of PWM count (ARR value)
m = 0.02  # motor mass
g = 9.81
T = 0.01
l_arm = 0.18

# arm parameters
l_cm = 0.135
m_cm = 1.3 * m
I_arm = m_cm * l_cm ** 2

# proportionality between angular and adc count accelleration
ADCMax = 4096
SensorFullAngle = 5.76  # radians
eta = ADCMax / SensorFullAngle

# relation between motor command and torque
gamma = m_cm * g * l_cm / (0.2 * TimerMax)

# simplifying constant
beta = T * T * gamma * eta / I_arm


def z_roots(kp, ki, kd):
    c4 = 1
    c3 = -3
    c2 = 3 + beta * ki * T + beta * kp + beta * kd / T
    c1 = -(1 + beta * kp + 2 * beta * kd / T)
    c0 = beta * kd / T
    coeff = [c4, c3, c2, c1, c0]

    roots = numpy.roots(coeff)
    return roots


def mag_z_roots(kp, ki, kd):
    return map(abs, z_roots(kp, ki, kd))


def max_z_root(ks):
    kp = ks[0]
    ki = ks[1]
    kd = ks[2]
    return max(mag_z_roots(kp, ki, kd))


#kp_array = numpy.linspace(0, 0.2, 400)
#ki_array = numpy.linspace(0, 0.01, 400)

#abs_roots = [[0] * len(kp_array) for _ in range(len(ki_array))]
#for i, ki in enumerate(ki_array):
#    for j, kp in enumerate(kp_array):
#        abs_roots[i][j] = max_z_root(ki, kp)

#plt.contourf(kp_array, ki_array, abs_roots, 100)
#plt.colorbar()
#plt.xlabel("kp")
#plt.ylabel("ki")
#plt.show()

print(scipy.optimize.minimize(max_z_root, [1, 1, 0.5], method='Nelder-Mead', tol=1e-10, options={'maxiter': 10000, 'maxfev': 10000}))