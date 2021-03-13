---
title: 2.1 Fem Pos Deviation Smoother
date: 2021-03-13 19:11:43
tags: Apollo
---

目录：modules/planning/math/discretized_points_smoothing/fem_pos_deviation_osqp_interface.cc

平滑类比较独立，所以单独摘出来看

设计了三个代价函数，分别代表了平滑性、曲线总长度和参考点距离

<!-- more -->

另外注意到，大部分场景都是只考虑平滑性（平滑性权重默认设置为1e7，其他的权重设置为1）

[只有在OpenSpacePlanner中，多考虑了一点到原参考点距离，权重设置的也比较低，只有1e3](https://github.com/ApolloAuto/apollo/commit/3decb9251da7e10b8ddf730c92cb1834a551b099)

因此下方，以6个点P0,P1,P2...P6为例子，只考虑平滑性代价时：
$cost = \sum_{i=0}^{n-2}((x_i + x_{i+2}-2\times x_{i+1})^2 + (y_i + y_{i+2}-2\times y_{i+1})^2)$
将上方cost乘开，转为QP公式
$$
\frac{1}{2}\cdot x^T\cdot Q\cdot x+p^T\cdot x\\
s.t. LB \leq A\cdot x \leq UB
$$
没有一次项，p矩阵为0
Q矩阵如下：
$$
\begin{vmatrix}
1 & -2 & 1 & 0 & 0 & 0\\ 
-2 & 5 & -4 & 1 & 0 & 0\\ 
1 & -4 & 6 & -4 & 1 & 0\\ 
0 & 1 & -4 & 6 & -4 & 1\\ 
0 & 0 & 1 & -4 & 5 & -2\\ 
0 & 0 & 0 & 1 & 2 & 1
\end{vmatrix}
$$

LB和UB是根据左右道路宽度来取的
设置完成后调用osqp求解器进行求解

除了平滑性，还配置了曲线总长度和参考线距离两个代价函数。
曲线总长度代价，使用积分进行运算：
$cost = \sum_{i=0}^{n-1}(x_i - x_{i+1})^2 + (y_i - y_{i+1})^2$
转为QP公式，一次项为0
二次项矩阵：
$$
\begin{vmatrix}
1 & -2 & 0 & 0 & 0 & 0\\ 
0 & 2 & -2 & 0 & 0 & 0\\ 
0 & 0 & 2 & -2 & 0 & 0\\ 
0 & 0 & 0 & 2 & -2 & 0\\ 
0 & 0 & 0 & 0 & 2 & -2\\ 
0 & 0 & 0 & 0 & 0 & 1
\end{vmatrix}
$$

到原参考点的距离：
$cost = \sum_{i=0}^{n-1}(x_i - x_{ref})^2 + (y_i - y_{ref})^2$
Q矩阵：
$$
\begin{vmatrix}
1 & 0 & 0 & 0 & 0 & 0\\ 
0 & 1 & 0 & 0 & 0 & 0\\ 
0 & 0 & 1 & 0 & 0 & 0\\ 
0 & 0 & 0 & 1 & 0 & 0\\ 
0 & 0 & 0 & 0 & 1 & 0\\ 
0 & 0 & 0 & 0 & 0 & 1
\end{vmatrix}
$$
q矩阵：
$$
\begin{vmatrix}
-2\cdot x_{ref1}\\
-2\cdot x_{ref2}\\
-2\cdot x_{ref3}\\
-2\cdot x_{ref4}\\
-2\cdot x_{ref5}\\
-2\cdot x_{ref6}
\end{vmatrix}
$$