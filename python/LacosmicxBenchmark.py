from argparse import ArgumentParser
import lacosmicx as lac
import numpy as np
import os
import time


def make_2d(side):
    image = np.empty([side, side], dtype=np.float32)
    for i in range(image.shape[0]):
        for j in range(image.shape[1]):
            image[i, j] = i + j
    return image


def print_2d(name, image):
    print(f"{name}:")
    print(f"  {image.shape[1]} x {image.shape[0]}")
    print(f"  [{image[0,0]}, ... , {image[-1,-1]}]")


if __name__ == "__main__":

    parser = ArgumentParser()
    parser.add_argument("--image", type=int, default=4000)
    args = parser.parse_args()

    print(f"OMP_NUM_THREADS: {os.environ['OMP_NUM_THREADS']}")

    print("Generating input...")
    input = make_2d(args.image)
    print_2d("input", input)

    print("Detecting...")
    start = time.perf_counter()
    crmask, cleanarr = lac.lacosmicx(indat=input, sepmed=False, verbose=True)
    stop = time.perf_counter()
    print(f"  Done in {stop-start} s")
    print_2d("crmask", crmask)
    print_2d("cleanarr", cleanarr)
