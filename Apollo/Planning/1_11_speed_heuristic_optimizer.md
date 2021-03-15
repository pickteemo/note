DP

参考https://blog.csdn.net/qq_41324346/article/details/105285029

### Fallback

首先做个保护，遍历障碍物的boundaries，如果起点有碰撞就返回fallback（车辆s、v、a都设置为0）

### InitCostTable

接下来初始化cost table，在该DP problem中，使用一个双层vector表示cost table，内层是s，外层是t

```c++
cost_table_ = std::vector<std::vector<StGraphPoint>>(
      dimension_t_, std::vector<StGraphPoint>(dimension_s_, StGraphPoint()));
```

首先看时间t维度，比较简单，默认配置是按7s，间隔为1s，均匀分布

然后看距离s维度，在前dense_dimension_s_（10）米，间隔为0.1m，之后间隔为1m，撒点

### InitSpeedLimitLookUp

获取每个s值对应的限速

```c++
  for (uint32_t i = 0; i < dimension_s_; ++i) {
    speed_limit_by_index_[i] =
        speed_limit.GetSpeedLimitByS(cost_table_[0][i].point().s());
  }
```

### CalculateTotalCost

遍历计算cost

cost = Obstacle cost + spatial_potential_cost + init_total_cost + v/a/j_cost

Obstacle cost:

1. st-bound之外的认为是无效点（无法抵达），cost为无穷大
2. 遍历障碍物的bound，计算障碍物boundary附近的cost（weight * (s_diff)^2 ）。(跟车或超车场景)

spatial_potential_cost:设置距离惩罚（保证到达终点）

init_total_cost:起点cost

v/a/j_cost:起点到该点的速度、加速度、加加速度cost

### RetrieveSpeedProfile

遍历找每个t上cost最小的点，然后计算速度