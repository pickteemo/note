---
title: 1.12 Speed Decider
date: 2021-03-18 11:44:47
tags: Apollo
---
前面通过DP获得了一个大致的速度折线，在时间上没有和障碍物发生碰撞。

接下来判断障碍物相对于该速度曲线的位置，

在曲线“上方”，需要判断是否需要避让来保持安全距离

在曲线“下方”，需要判断超车处理

<!-- more -->

遍历obs，依次判断以下条件：

1. 忽略掉空间或者时间上不相关的obs

```c++
    if (boundary.IsEmpty() || boundary.max_s() < 0.0 ||
        boundary.max_t() < 0.0 ||
        boundary.min_t() >= speed_profile.back().t()) {
      AppendIgnoreDecision(mutable_obstacle);
      continue;
    }
```

2. 忽略掉已经有纵向决策的obs

```c++
    if (obstacle->HasLongitudinalDecision()) {
      AppendIgnoreDecision(mutable_obstacle);
      continue;
    }
```

3. 忽略掉不相关的虚拟的obs

```c++
    // for Virtual obstacle, skip if center point NOT "on lane"
    if (obstacle->IsVirtual()) {
      const auto& obstacle_box = obstacle->PerceptionBoundingBox();
      if (!reference_line_->IsOnLane(obstacle_box.center())) {
        continue;
      }
    }
```

4. 遇到人行横道，需要停下来观察一下

```c++
    // always STOP for pedestrian
    if (CheckStopForPedestrian(*mutable_obstacle)) {
      ObjectDecisionType stop_decision;
      if (CreateStopDecision(*mutable_obstacle, &stop_decision,
                             -FLAGS_min_stop_distance_obstacle)) {
        mutable_obstacle->AddLongitudinalDecision("dp_st_graph/pedestrian",
                                                  stop_decision);
      }
      continue;
    }
```

5. 剩下的常规处理，先判断自车boundary是在上方还是下方

   1. 自车在boundary的下方，依次检查stop、follow、yield

   ```c++
   case BELOW:
           if (boundary.boundary_type() == STBoundary::BoundaryType::KEEP_CLEAR) {
             ObjectDecisionType stop_decision;
             if (CreateStopDecision(*mutable_obstacle, &stop_decision, 0.0)) {
               mutable_obstacle->AddLongitudinalDecision("dp_st_graph/keep_clear",
                                                         stop_decision);
             }
           } else if (CheckIsFollow(*obstacle, boundary)) {
             // stop for low_speed decelerating
             if (IsFollowTooClose(*mutable_obstacle)) {
               ObjectDecisionType stop_decision;
               if (CreateStopDecision(*mutable_obstacle, &stop_decision,
                                      -FLAGS_min_stop_distance_obstacle)) {
                 mutable_obstacle->AddLongitudinalDecision("dp_st_graph/too_close",
                                                           stop_decision);
               }
             } else {  // high speed or low speed accelerating
               // FOLLOW decision
               ObjectDecisionType follow_decision;
               if (CreateFollowDecision(*mutable_obstacle, &follow_decision)) {
                 mutable_obstacle->AddLongitudinalDecision("dp_st_graph",
                                                           follow_decision);
               }
             }
           } else {
             // YIELD decision
             ObjectDecisionType yield_decision;
             if (CreateYieldDecision(*mutable_obstacle, &yield_decision)) {
               mutable_obstacle->AddLongitudinalDecision("dp_st_graph",
                                                         yield_decision);
             }
           }
           break;
   ```

   2. 自车在boundary的上方，检查overtake

   ```c++
   case ABOVE:
           if (boundary.boundary_type() == STBoundary::BoundaryType::KEEP_CLEAR) {
             ObjectDecisionType ignore;
             ignore.mutable_ignore();
             mutable_obstacle->AddLongitudinalDecision("dp_st_graph", ignore);
           } else {
             // OVERTAKE decision
             ObjectDecisionType overtake_decision;
             if (CreateOvertakeDecision(*mutable_obstacle, &overtake_decision)) {
               mutable_obstacle->AddLongitudinalDecision("dp_st_graph/overtake",
                                                         overtake_decision);
             }
           }
           break;
   ```

   3. 有交叉，需要停车处理

   ```c++
   case CROSS:
           if (mutable_obstacle->IsBlockingObstacle()) {
             ObjectDecisionType stop_decision;
             if (CreateStopDecision(*mutable_obstacle, &stop_decision,
                                    -FLAGS_min_stop_distance_obstacle)) {
               mutable_obstacle->AddLongitudinalDecision("dp_st_graph/cross",
                                                         stop_decision);
             }
             const std::string msg = absl::StrCat(
                 "Failed to find a solution for crossing obstacle: ",
                 mutable_obstacle->Id());
             AERROR << msg;
             return Status(ErrorCode::PLANNING_ERROR, msg);
           }
           break;
   ```

   

