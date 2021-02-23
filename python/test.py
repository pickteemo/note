import osqp
import numpy as np
import matplotlib.pyplot as plt
from scipy import sparse

x_array = [0.5, 1.0, 2.0, 3.0, 4.0,
           5.0, 6.0, 7.0, 8.0, 9.0]
y_array = [0.1, 0.3, 0.2, 0.4, 0.3,
           -0.2, -0.1, 0, 0.5, 0]
print(len(x_array), len(y_array))
plt.plot(x_array, y_array)

length = len(x_array)

weight_fem_pos_deviation_ = 1e7  # cost1 - x
weight_path_length = 1  # cost2 - y
weight_ref_deviation = 1  # cost3 - z
bound = 0.1

P = np.zeros((length, length))
q = np.ones(length)
# add cost1
P[0, 0] = 1 * weight_fem_pos_deviation_
P[0, 1] = -2 * weight_fem_pos_deviation_
P[1, 1] = 5 * weight_fem_pos_deviation_
P[length - 1, length - 1] = 1 * weight_fem_pos_deviation_
P[length - 2, length - 1] = -2 * weight_fem_pos_deviation_
P[length - 2, length - 2] = 5 * weight_fem_pos_deviation_

# for i in range(2, length - 2):
#     P[i, i] = 6 * weight_fem_pos_deviation_
# for i in range(2, length - 1):
#     P[i - 1, i] = -4 * weight_fem_pos_deviation_
# for i in range(2, length):
#     P[i - 2, i] = 1 * weight_fem_pos_deviation_

P = P / weight_fem_pos_deviation_
print(P)

A = np.zeros((length, length))
for i in range(length):
    A[i, i] = 1
print(A)

P = sparse.csc_matrix(P)
A = sparse.csc_matrix(A)
l = np.array(x_array) - bound
u = np.array(x_array) + bound
print(P)
print(q)
print(A)
print(l)
print(u)

# P1 = sparse.csc_matrix([[4, 1], [1, 2]])
# q1 = np.array([1, 1])
# A = sparse.csc_matrix([[1], [1], [0]])
# l = np.array([1, 5, 10])
# u = np.array([1, 6, 20])

# prob1 = osqp.OSQP()
# prob1.setup(P1, q1, A1, l1, u1, alpha=1.0)
# res1 = prob1.solve()
# print(res1.x, res1.y)


prob = osqp.OSQP()
prob.setup(P, q, A, l, u)
res = prob.solve()
print(res.x, res.y)
