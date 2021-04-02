---
title: 1.3 Path Lane Borrow Decider
date: 2021-03-09 19:11:43
tags: Apollo
---

> Path Lane Borrow Decider

<!-- more -->



首先看一下自车当前的状态path_decider_status

```
auto* mutable_path_decider_status = injector_->planning_context()
                                          ->mutable_planning_status()
                                          ->mutable_path_decider();
```
path_decider_status是定义在proto中的结构体(modules/planning/proto/planning_status.proto),包括前方obs的counter、id，以及允许借道的方向等
```
message PathDeciderStatus {
  enum LaneBorrowDirection {
    LEFT_BORROW = 1;   // borrow left neighbor lane
    RIGHT_BORROW = 2;  // borrow right neighbor lane
  }
  optional int32 front_static_obstacle_cycle_counter = 1 [default = 0];
  optional int32 able_to_use_self_lane_counter = 2 [default = 0];
  optional bool is_in_path_lane_borrow_scenario = 3 [default = false];
  optional string front_static_obstacle_id = 4 [default = ""];
  repeated LaneBorrowDirection decided_side_pass_direction = 5;
}
```
如果当前已经在借道状态了，并且自车车道可用的counter >=6，取消lane_borrow scenario 状态
```
if (mutable_path_decider_status->able_to_use_self_lane_counter() >= 6) {
      // If have been able to use self-lane for some time, then switch to
      // non-lane-borrowing.
      mutable_path_decider_status->set_is_in_path_lane_borrow_scenario(false);
      mutable_path_decider_status->clear_decided_side_pass_direction();
      AINFO << "Switch from LANE-BORROW path to SELF-LANE path.";
    }
```
然后是一串黑名单，用于筛除不需要借道的场景，判断是否确实需要换道
只有当以下条件都满足时，才认为需要借道：
- HasSingleReferenceLine:只有一条参考线


```

```
`frame.reference_line_info().size() == 1;`
- IsWithinSidePassingSpeedADC:自车车速较低 (18km/h)
`frame.PlanningStartPoint().v() < FLAGS_lane_borrow_max_speed`
- IsBlockingObstacleFarFromIntersection：参考线被障碍物遮挡，并且障碍物不在特殊区域（路口、人行横道等）附近（20m）
- IsLongTermBlockingObstacle：长期遮挡参考线（counter >=3）
`front_static_obstacle_cycle_counter() >=FLAGS_long_term_blocking_obstacle_cycle_threshold`
- IsBlockingObstacleWithinDestination:障碍物不在终点附近
- IsSidePassableObstacle：障碍物最终确认，在modules/planning/common/obstacle_blocking_analyzer.cc :bool IsNonmovableObstacle函数中，包括：
    - 前方最近的障碍物距离自车不是很远（35m）
    `obstacle.PerceptionSLBoundary().start_s() > adc_sl_boundary.end_s() + kAdcDistanceThreshold`
    - 前方最近的障碍物在路边，或者是在停车道上
    ```
    bool is_parked = is_on_parking_lane || is_at_road_edge;
    return is_parked && obstacle->IsStatic();
    ```
    - 前方最近的障碍物的前方（15m内）没有其他障碍物
对应代码：
```
    // ADC requirements check for lane-borrowing:
    if (!HasSingleReferenceLine(frame)) {
      return false;
    }
    if (!IsWithinSidePassingSpeedADC(frame)) {
      return false;
    }

    // Obstacle condition check for lane-borrowing:
    if (!IsBlockingObstacleFarFromIntersection(reference_line_info)) {
      return false;
    }
    if (!IsLongTermBlockingObstacle()) {
      return false;
    }
    if (!IsBlockingObstacleWithinDestination(reference_line_info)) {
      return false;
    }
    if (!IsSidePassableObstacle(reference_line_info)) {
      return false;
    }
```

满足了以上条件后，首先检查左右的车道线是否为实线和双黄线
```
    if (*left_neighbor_lane_borrowable) {
      lane_boundary_type = hdmap::LeftBoundaryType(waypoint);
      if (lane_boundary_type == hdmap::LaneBoundaryType::SOLID_YELLOW ||
          lane_boundary_type == hdmap::LaneBoundaryType::SOLID_WHITE) {
        *left_neighbor_lane_borrowable = false;
      }
      ADEBUG << "s[" << check_s << "] left_lane_boundary_type["
             << LaneBoundaryType_Type_Name(lane_boundary_type) << "]";
    }
    if (*right_neighbor_lane_borrowable) {
      lane_boundary_type = hdmap::RightBoundaryType(waypoint);
      if (lane_boundary_type == hdmap::LaneBoundaryType::SOLID_YELLOW ||
          lane_boundary_type == hdmap::LaneBoundaryType::SOLID_WHITE) {
        *right_neighbor_lane_borrowable = false;
      }
      ADEBUG << "s[" << check_s << "] right_neighbor_lane_borrowable["
             << LaneBoundaryType_Type_Name(lane_boundary_type) << "]";
    }
```
将可以借道的方向添加到decided_side_pass_direction中，is_in_path_lane_borrow_scenario置为True
如果两个方向都不能借道，is_in_path_lane_borrow_scenario置为false
```
if (path_decider_status.decided_side_pass_direction().empty()) {
      // first time init decided_side_pass_direction
      bool left_borrowable;
      bool right_borrowable;
      CheckLaneBorrow(reference_line_info, &left_borrowable, &right_borrowable);
      if (!left_borrowable && !right_borrowable) {
        mutable_path_decider_status->set_is_in_path_lane_borrow_scenario(false);
        return false;
      } else {
        mutable_path_decider_status->set_is_in_path_lane_borrow_scenario(true);
        if (left_borrowable) {
          mutable_path_decider_status->add_decided_side_pass_direction(
              PathDeciderStatus::LEFT_BORROW);
        }
        if (right_borrowable) {
          mutable_path_decider_status->add_decided_side_pass_direction(
              PathDeciderStatus::RIGHT_BORROW);
        }
      }
    }
```