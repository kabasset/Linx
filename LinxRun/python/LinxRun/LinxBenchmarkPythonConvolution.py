import argparse
import numpy as np
from scipy import ndimage
import time


def defineSpecificProgramOptions():
    parser = argparse.ArgumentParser()
    parser.add_argument('--image', type=int, default=2048)
    parser.add_argument('--kernel', type=int, default=5)
    parser.add_argument('--extrapolation', default='nearest')
    return parser


def mainMethod(args):

    image_diameter = args.image
    kernel_diameter = args.kernel
    extrapolation = args.extrapolation
    image_shape = (image_diameter, image_diameter)
    kernel_shape = (kernel_diameter, kernel_diameter)

    print('Generating raster and kernel...')
    image = np.arange(np.prod(image_shape), dtype=np.float32).reshape(image_shape)
    kernel = np.arange(np.prod(kernel_shape), dtype=np.float32).reshape(kernel_shape)
    print(f'  input: {image}')

    print('Filtering...')
    start = time.time()
    ndimage.convolve(image, kernel, output=image, mode=extrapolation)
    end = time.time()
    print(f'  output: {image}')

    print(f'  Done in: {(end - start) * 1000} ms')
