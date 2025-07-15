# This is the same code as 107-central-differences.tachyon, but in Python. For benchmarking
# purposes.
#
# On my laptop, the Tachyon version took 3m42.379s (pre-jit, no memoization) =  222.379s.
#               the Python  version took 51m16.883s                          = 3076.883s.

import numpy as np


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


def cost(weights, biases, layer_data, layer_sizes, layer_amount, do_print):
    total_cost = 0

    for y in range(30):
        for x in range(30):
            layer_data[:] = 0
            layer_data[0, 0] = x
            layer_data[0, 1] = y

            for i in range(layer_amount):
                for j in range(layer_sizes[i + 1]):
                    total = 0
                    for k in range(layer_sizes[i]):
                        total += layer_data[i, k] * weights[k, i * 2 + j]
                    total += biases[j, i * 2 + j]
                    layer_data[i + 1, j] = sigmoid(total)

            expected_val = 1 if x < 5 or y < 5 else 0

            if do_print:
                output1 = layer_data[layer_amount, 0]
                output2 = layer_data[layer_amount, 1]
                if output1 >= output2:
                    print("S ", end="")
                else:
                    print("F ", end="")

            val1 = (layer_data[layer_amount, 0] - expected_val) ** 2
            val2 = (layer_data[layer_amount, 1] - (1 - expected_val)) ** 2
            total_cost += val1 + val2

        if do_print:
            print()

    return total_cost


weights = np.array([
    [1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0],
    [-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0]
])

biases = np.array([
    [-5.0, -5.0, 0, 0, 0, 0, 0, 0, 0, 0],
    [-5.0, -5.0, 0, 0, 0, 0, 0, 0, 0, 0]
])

layer_data = np.zeros((6, 2))
layer_sizes = [2, 2, 2, 2, 2, 2]
layer_amount = 5
learning_rate = 0.005
h = 0.00001

print("starting cost:")
gen = cost(weights.copy(), biases.copy(), layer_data.copy(), layer_sizes, layer_amount, True)
print(gen)
print()

for gens in range(1, 101):
    for i in range(layer_amount):
        for j in range(2):
            for k in range(10):
                weights[j, k] -= h
                gen1 = cost(weights, biases, layer_data.copy(), layer_sizes, layer_amount, False)
                weights[j, k] += 2 * h
                gen2 = cost(weights, biases, layer_data.copy(), layer_sizes, layer_amount, False)
                delta = gen1 - gen2
                slope = delta / (2 * h)
                weights[j, k] += slope * learning_rate

        for j in range(2):
            for k in range(10):
                gen1 = cost(weights, biases, layer_data.copy(), layer_sizes, layer_amount, False)
                biases[j, k] += h
                gen2 = cost(weights, biases, layer_data.copy(), layer_sizes, layer_amount, False)
                biases[j, k] -= h
                delta = gen1 - gen2
                slope = delta / h
                weights[j, k] += slope * learning_rate

    gen = cost(weights, biases, layer_data.copy(), layer_sizes, layer_amount, True)
    print("current cost:")
    print(gen)
    print()
