---
title: 1.10 Speed Bounds Prior Decider
date: 2021-03-13 19:11:43
tags: Apollo
---




这个task会执行两次，主要业务是把障碍物投到st图，再设置path上的速度限制。

第一次跑不涉及障碍物在速度方面的decision，第二次是在添加decision之后，需要考虑decision。

<!-- more -->

### Map obstacles into st graph

这里上来就搞了一个STBoundaryMapper Object，获取参考线、配置、path decision、规划长度和时间等参数。

然后ComputeSTBoundary，区分了几种情况: no Longitudinal Decision和has Longitudinal Decision(has stop、has follow、has overtake、has yield、has ignore)，在prior中只有no decision和has stop，

```c++
  STBoundaryMapper boundary_mapper(
      speed_bounds_config_, reference_line, path_data,
      path_data.discretized_path().Length(), speed_bounds_config_.total_time(),
      injector_);
  if (!FLAGS_use_st_drivable_boundary) {
    path_decision->EraseStBoundaries();
  }

  if (boundary_mapper.ComputeSTBoundary(path_decision).code() ==
      ErrorCode::PLANNING_ERROR) {
    const std::string msg = "Mapping obstacle failed.";
    AERROR << msg;
    return Status(ErrorCode::PLANNING_ERROR, msg);
  }
```

bound的生成：

```c++
  auto boundary = STBoundary::CreateInstance(lower_points, upper_points);
```

这里使用lower_points和upper_points生成STBoundary（Polygon），用散点生成一个由line segment组成的框（Polygon）。

流程：

首先做稀疏化(RemoveRedundantPoints)，保证点与点的距离大于0.1(每个小框的面积>= 0.01)

然后和之前一样，调用BuildFromPoints，连接point得到line segment，由segment包成一个polygon



### GetSpeedLimits

障碍物的ST Boundary得到后，遍历计算每个s对应的speedlimit，来源包括：

1、Map - 地图上标的限速

2、Curvature - sqrt(横向加速度 a / 道路曲率 kappa)

3、NudgeObs - 对nudge过程中过近的obs做限速处理。在frenet坐标系下，当自车与obs在s方向有重叠，l方向距离小于safe_range(1.0)时，做限速处理：静态障碍物的限速为0.6×Map限速，动态为0.8×Map限速



