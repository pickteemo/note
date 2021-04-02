---
title: 1.9 ST Bound Decider
date: 2021-03-15 19:11:43
tags: Apollo
---


<!-- more -->

### 初始化ST-Graph

构造场景如下图：

[![yWkQP0.png](https://s3.ax1x.com/2021/02/18/yWkQP0.png)](https://imgchr.com/i/yWkQP0)



#### 将Obstacle转为ST Boundaries

遍历障碍物的预测轨迹点，与之前生成的轨迹线做碰撞检测，记录碰撞时间与位置（t-s_upper,t-s_lower）

```c++
for (const auto& obs_traj_pt : obs_trajectory.trajectory_point()) {
      // TODO(jiacheng): Currently, if the obstacle overlaps with ADC at
      // disjoint segments (happens very rarely), we merge them into one.
      // In the future, this could be considered in greater details rather
      // than being approximated.
      const Box2d& obs_box = obstacle.GetBoundingBox(obs_traj_pt);
      ADEBUG << obs_box.DebugString();
      std::pair<double, double> overlapping_s;
      if (GetOverlappingS(adc_path_points, obs_box, kADCSafetyLBuffer,
                          &overlapping_s)) {
        ADEBUG << "Obstacle instance is overlapping with ADC path.";
        lower_points->emplace_back(overlapping_s.first,
                                   obs_traj_pt.relative_time());
        upper_points->emplace_back(overlapping_s.second,
                                   obs_traj_pt.relative_time());
        if (is_obs_first_traj_pt) {
          if (IsSWithinADCLowRoadRightSegment(overlapping_s.first) ||
              IsSWithinADCLowRoadRightSegment(overlapping_s.second)) {
            *is_caution_obstacle = true;
          }
        }
        if ((*is_caution_obstacle)) {
          if (IsSWithinADCLowRoadRightSegment(overlapping_s.first) ||
              IsSWithinADCLowRoadRightSegment(overlapping_s.second)) {
            *obs_caution_end_t = obs_traj_pt.relative_time();
          }
        }
      }
      is_obs_first_traj_pt = false;
    }
```

使用上述记录的离散点（s和t）构建STBoundary，将符合st图条件（在图内）的boundary存下来。

STBoundary是基于Polygon的多边形类，如下图：

![img](https://img2018.cnblogs.com/blog/1702455/201906/1702455-20190619150346811-127762010.png)

```c++
    auto boundary =
        STBoundary::CreateInstanceAccurate(lower_points, upper_points);
```

```c++
// Process all other obstacles than Keep-Clear zone.
    if (obs_ptr->Trajectory().trajectory_point().empty()) {
      // Obstacle is static.
      if (std::get<0>(closest_stop_obstacle) == "NULL" ||
          std::get<1>(closest_stop_obstacle).bottom_left_point().s() >
              boundary.bottom_left_point().s()) {
        // If this static obstacle is closer for ADC to stop, record it.
        closest_stop_obstacle =
            std::make_tuple(obs_ptr->Id(), boundary, obs_ptr);
      }
    } else {
      // Obstacle is dynamic.
      if (boundary.bottom_left_point().s() - adc_path_init_s_ <
              kSIgnoreThreshold &&
          boundary.bottom_left_point().t() > kTIgnoreThreshold) {
        // Ignore obstacles that are behind.
        // TODO(jiacheng): don't ignore if ADC is in dangerous segments.
        continue;
      }
      obs_id_to_st_boundary_[obs_ptr->Id()] = boundary;
      obs_ptr->set_path_st_boundary(boundary);
      non_ignore_obstacles.insert(obs_ptr->Id());
      ADEBUG << "Adding " << obs_ptr->Id() << " into the ST-graph.";
    }
```

静态障碍物只考虑最近的和Keep-Clear 区域内的

```c++
  // For static obstacles, only retain the closest one (also considers
  // Keep-Clear zone here).
  // Note: We only need to check the overlapping between the closest obstacle
  //       and all the Keep-Clear zones. Because if there is another obstacle
  //       overlapping with a Keep-Clear zone, which results in an even closer
  //       stop fence, then that very Keep-Clear zone must also overlap with
  //       the closest obstacle. (Proof omitted here)
  if (std::get<0>(closest_stop_obstacle) != "NULL") {
    std::string closest_stop_obs_id;
    STBoundary closest_stop_obs_boundary;
    Obstacle* closest_stop_obs_ptr;
    std::tie(closest_stop_obs_id, closest_stop_obs_boundary,
             closest_stop_obs_ptr) = closest_stop_obstacle;
    ADEBUG << "Closest obstacle ID = " << closest_stop_obs_id;
    // Go through all Keep-Clear zones, and see if there is an even closer
    // stop fence due to them.
    if (!closest_stop_obs_ptr->IsVirtual()) {
      for (const auto& clear_zone : candidate_clear_zones_) {
        const auto& clear_zone_boundary = std::get<1>(clear_zone);
        if (closest_stop_obs_boundary.min_s() >= clear_zone_boundary.min_s() &&
            closest_stop_obs_boundary.min_s() <= clear_zone_boundary.max_s()) {
          std::tie(closest_stop_obs_id, closest_stop_obs_boundary,
                   closest_stop_obs_ptr) = clear_zone;
          ADEBUG << "Clear zone " << closest_stop_obs_id << " is closer.";
          break;
        }
      }
    }
    obs_id_to_st_boundary_[closest_stop_obs_id] = closest_stop_obs_boundary;
    closest_stop_obs_ptr->set_path_st_boundary(closest_stop_obs_boundary);
    non_ignore_obstacles.insert(closest_stop_obs_id);
    ADEBUG << "Adding " << closest_stop_obs_ptr->Id() << " into the ST-graph.";
    ADEBUG << "min_s = " << closest_stop_obs_boundary.min_s();
  }
```

剩余的障碍物添加ignore tag

与path plan类似，obs_t_edges中存放了障碍物的起点(True)和终点(False)的时间（t）和位置（沿s方向的upper，bottom ）信息。

最后按照t的顺序将boundary进行排序，完成obs的boundary的生成。

```c++
  // Preprocess the obstacles for sweep-line algorithms.
  // Fetch every obstacle's beginning end ending t-edges only.
  for (const auto& it : obs_id_to_st_boundary_) {
    obs_t_edges_.emplace_back(true, it.second.min_t(),
                              it.second.bottom_left_point().s(),
                              it.second.upper_left_point().s(), it.first);
    obs_t_edges_.emplace_back(false, it.second.max_t(),
                              it.second.bottom_right_point().s(),
                              it.second.upper_right_point().s(), it.first);
  }
  // Sort the edges.
  std::sort(obs_t_edges_.begin(), obs_t_edges_.end(),
            [](const ObsTEdge& lhs, const ObsTEdge& rhs) {
              if (std::get<1>(lhs) != std::get<1>(rhs)) {
                return std::get<1>(lhs) < std::get<1>(rhs);
              } else {
                return std::get<0>(lhs) > std::get<0>(rhs);
              }
            });
```



### GenerateRegularSTBound

上面生成的obs boundary，在同一时刻可能会有很多组（每个obs对应一组），需要进行处理得到合适的bound。

与path中的GenerateRegularSLBound类似，使用向前扫的方式(sweep-line)获得具体的ST-boundary。

#### 初始化

按0.1s的步长初始化st - bound

```c++
for (double curr_t = 0.0; curr_t <= st_bounds_config_.total_time();
       curr_t += kSTBoundsDeciderResolution) {
    st_bound->emplace_back(curr_t, std::numeric_limits<double>::lowest(),
                           std::numeric_limits<double>::max());
    vt_bound->emplace_back(curr_t, std::numeric_limits<double>::lowest(),
                           std::numeric_limits<double>::max());
  }
```

#### Vehicle Dynamics Limits运动学限制

根据车辆运动学限制获得t时刻沿s方向的bound

```c++
    // Get Boundary due to driving limits
    auto driving_limits_bound = st_driving_limits_.GetVehicleDynamicsLimits(t);
    s_lower = std::fmax(s_lower, driving_limits_bound.first);
    s_upper = std::fmin(s_upper, driving_limits_bound.second);
    ADEBUG << "Bounds for s due to driving limits are "
           << "s_upper = " << s_upper << ", s_lower = " << s_lower;

```



#### Obstacle 障碍物

计算t时刻的obs boundary,获得s方向的bound和obs对应的decision

```c++
// Get Boundary due to obstacles
    std::vector<std::pair<double, double>> available_s_bounds;
    std::vector<ObsDecSet> available_obs_decisions;
    if (!st_obstacles_processor_.GetSBoundsFromDecisions(
            t, &available_s_bounds, &available_obs_decisions)) {
      const std::string msg =
          "Failed to find a proper boundary due to obstacles.";
      AERROR << msg;
      return Status(ErrorCode::PLANNING_ERROR, msg);
    }
    std::vector<std::pair<STBoundPoint, ObsDecSet>> available_choices;
    ADEBUG << "Available choices are:";
    for (int j = 0; j < static_cast<int>(available_s_bounds.size()); ++j) {
      ADEBUG << "  (" << available_s_bounds[j].first << ", "
             << available_s_bounds[j].second << ")";
      available_choices.emplace_back(
          std::make_tuple(0.0, available_s_bounds[j].first,
                          available_s_bounds[j].second),
          available_obs_decisions[j]);
    }
    RemoveInvalidDecisions(driving_limits_bound, &available_choices);
```

主要的功能实现在GetSBoundsFromDecisions函数中,遍历比较t时刻的障碍物的smax,smin与自车的smax,smin，给每个障碍物打超车、避让tag。

最后remove掉不合理的decision（driving limits bound）

#### Make Obstacle Final Decision

比较guide line和boundary，按规则对obs的Decision进行排序（RankDecisions）

做出t时刻的超车(Overtake)/减速避让(Yield)相关的最终Decision

根据t时刻的s方向的边界和决策，求导计算v的边界

```c++
if (!available_choices.empty()) {
      ADEBUG << "One decision needs to be made among "
             << available_choices.size() << " choices.";
      double guide_line_s = st_guide_line_.GetGuideSFromT(t);
      st_guide_line->emplace_back(t, guide_line_s);
      RankDecisions(guide_line_s, driving_limits_bound, &available_choices);
      // Select the top decision.
      auto top_choice_s_range = available_choices.front().first;
      bool is_limited_by_upper_obs = false;
      bool is_limited_by_lower_obs = false;
      if (s_lower < std::get<1>(top_choice_s_range)) {
        s_lower = std::get<1>(top_choice_s_range);
        is_limited_by_lower_obs = true;
      }
      if (s_upper > std::get<2>(top_choice_s_range)) {
        s_upper = std::get<2>(top_choice_s_range);
        is_limited_by_upper_obs = true;
      }

      // Set decision for obstacles without decisions.
      auto top_choice_decision = available_choices.front().second;
      st_obstacles_processor_.SetObstacleDecision(top_choice_decision);

      // Update st-guide-line, st-driving-limit info, and v-limits.
      std::pair<double, double> limiting_speed_info;
      if (st_obstacles_processor_.GetLimitingSpeedInfo(t,
                                                       &limiting_speed_info)) {
        st_driving_limits_.UpdateBlockingInfo(
            t, s_lower, limiting_speed_info.first, s_upper,
            limiting_speed_info.second);
        st_guide_line_.UpdateBlockingInfo(t, s_lower, true);
        st_guide_line_.UpdateBlockingInfo(t, s_upper, false);
        if (is_limited_by_lower_obs) {
          lower_obs_v = limiting_speed_info.first;
        }
        if (is_limited_by_upper_obs) {
          upper_obs_v = limiting_speed_info.second;
        }
      }
    } 
```

