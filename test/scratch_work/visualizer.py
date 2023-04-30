import numpy as np
import matplotlib.pyplot as plt

def visualize(rdm):
    # Only for a single channel, the C++ code performed averaging
    plt.imshow(rdm, origin='lower', aspect='auto')
    plt.show()

# Specify the dimensions of the array
RANGE_BINS=512
DOPPLER_BINS=64

rdm=np.loadtxt("out.txt")
assert rdm.shape == (64, 512)
rdm=np.fft.fftshift(rdm, axes=1)
print("It works I believe")
visualize(rdm)
