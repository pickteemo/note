---
title: 1.6 Path Assessment Decider
date: 2021-03-12 19:11:43
tags: Apollo
---

path optimizer根据boundaries生成了很多条空间上的path
在进行速度规划之前，需要先验证生成的path的合理性
<!-- more -->

file:modules/planning/tasks/deciders/path_assessment_decider/path_assessment_decider.cc

代码的注释比较清晰，decider主要分成四部分：
>1. Remove invalid path.
>2. Analyze and add important info for speed decider to use
>3. Pick the optimal path.
>4. Update necessary info for lane-borrow decider's future uses.

### 1.Remove invalid path
#### Is valid fallback path 
设置了3个条件来验证fallbackpath的合理性
- 轨迹点不为空
- 距离参考线不是特别远(20m)
- 距离左右车道线不是特别远（10m）

```
  // Basic sanity checks.
  if (path_data.Empty()) {
    ADEBUG << "Fallback Path: path data is empty.";
    return false;
  }
  // Check if the path is greatly off the reference line.
  if (IsGreatlyOffReferenceLine(path_data)) {
    ADEBUG << "Fallback Path: ADC is greatly off reference line.";
    return false;
  }
  // Check if the path is greatly off the road.
  if (IsGreatlyOffRoad(reference_line_info, path_data)) {
    ADEBUG << "Fallback Path: ADC is greatly off road.";
    return false;
  }
  return true;
```
#### Is valid regular path
设置了5个条件来验证常规path的合理性
- 轨迹点不为空
- 距离参考线不是特别远(20m)
- 距离左右车道线不是特别远（10m）
- 与静态障碍物没有发生碰撞：path不应当与静态障碍物发生碰撞
- 不会停在反向道路上：检查终点不在目标车道上
```
  // Basic sanity checks.
  if (path_data.Empty()) {
    ADEBUG << path_data.path_label() << ": path data is empty.";
    return false;
  }
  // Check if the path is greatly off the reference line.
  if (IsGreatlyOffReferenceLine(path_data)) {
    ADEBUG << path_data.path_label() << ": ADC is greatly off reference line.";
    return false;
  }
  // Check if the path is greatly off the road.
  if (IsGreatlyOffRoad(reference_line_info, path_data)) {
    ADEBUG << path_data.path_label() << ": ADC is greatly off road.";
    return false;
  }
  // Check if there is any collision.
  if (IsCollidingWithStaticObstacles(reference_line_info, path_data)) {
    ADEBUG << path_data.path_label() << ": ADC has collision.";
    return false;
  }

  if (IsStopOnReverseNeighborLane(reference_line_info, path_data)) {
    ADEBUG << path_data.path_label() << ": stop at reverse neighbor lane";
    return false;
  }
```

### 2.Analyze and add important info for speed decider to use
添加一些信息供speed decider使用

对于涉及换道的path，需要标记出换道点的位置（in lane 、 out lane）

对于block的轨迹，需要标记出block的位置

### 3. Pick the optimal path.
挑选最好的path，根据设置的规则对所有路径按优先级进行排序
```
  std::sort(valid_path_data.begin(), valid_path_data.end(),
            std::bind(ComparePathData, std::placeholders::_1,
                      std::placeholders::_2, blocking_obstacle_on_selflane));
```
看一下ComparePathData中设置的一些规则：
1. fallback的path放在最后：
```
  // Regular path goes before fallback path.
  bool lhs_is_regular = lhs.path_label().find("regular") != std::string::npos;
  bool rhs_is_regular = rhs.path_label().find("regular") != std::string::npos;
  if (lhs_is_regular != rhs_is_regular) {
    return lhs_is_regular;
  }
```
2. 挑选更长的path： 例如在借道避让场景下，向左避让一般比向右避让优先级更高；但是当右避让的path比左避让的path更长时（tolerance = 15m），优先选择右避让的path；对于换道的path，可以设置更高的tolerance（25m ）
```
  // Select longer path.
  // If roughly same length, then select self-lane path.
  bool lhs_on_selflane = lhs.path_label().find("self") != std::string::npos;
  bool rhs_on_selflane = rhs.path_label().find("self") != std::string::npos;
  static constexpr double kSelfPathLengthComparisonTolerance = 15.0;
  static constexpr double kNeighborPathLengthComparisonTolerance = 25.0;
  double lhs_path_length = lhs.frenet_frame_path().back().s();
  double rhs_path_length = rhs.frenet_frame_path().back().s();
  if (lhs_on_selflane || rhs_on_selflane) {
    if (std::fabs(lhs_path_length - rhs_path_length) >
        kSelfPathLengthComparisonTolerance) {
      return lhs_path_length > rhs_path_length;
    } else {
      return lhs_on_selflane;
    }
  } else {
    if (std::fabs(lhs_path_length - rhs_path_length) >
        kNeighborPathLengthComparisonTolerance) {
      return lhs_path_length > rhs_path_length;
    }
  }
```
3. 第二步留了一种换道的场景：自车必须要进行换道避障，左右避障的path长度又差不多，该怎么选？首先排除有一侧（一般左侧）有逆行的道路的情况
```
// If roughly same length, and must borrow neighbor lane,
  // then prefer to borrow forward lane rather than reverse lane.
  int lhs_on_reverse =
      ContainsOutOnReverseLane(lhs.path_point_decision_guide());
  int rhs_on_reverse =
      ContainsOutOnReverseLane(rhs.path_point_decision_guide());
  // TODO(jiacheng): make this a flag.
  if (std::abs(lhs_on_reverse - rhs_on_reverse) > 6) {
    return lhs_on_reverse < rhs_on_reverse;
  }
```
剩下的情况就是两边都可以避，path也差不多的情况。这种情况下尽量选择一条更为方便的路径（convenient path），如果当前的path有blocking Obstacle，那就看Obstacle的位置，Obstacle靠左就右避让，靠右的左避让；如果没有Obstacle，就选择较近的path进行避让（l值更小）。
```
/ For two lane-borrow directions, based on ADC's position,
  // select the more convenient one.
  if ((lhs.path_label().find("left") != std::string::npos &&
       rhs.path_label().find("right") != std::string::npos) ||
      (lhs.path_label().find("right") != std::string::npos &&
       rhs.path_label().find("left") != std::string::npos)) {
    if (blocking_obstacle) {
      // select left/right path based on blocking_obstacle's position
      const double obstacle_l =
          (blocking_obstacle->PerceptionSLBoundary().start_l() +
           blocking_obstacle->PerceptionSLBoundary().end_l()) /
          2;
      ADEBUG << "obstacle[" << blocking_obstacle->Id() << "] l[" << obstacle_l
             << "]";
      return (obstacle_l > 0.0
                  ? (lhs.path_label().find("right") != std::string::npos)
                  : (lhs.path_label().find("left") != std::string::npos));
    } else {
      // select left/right path based on ADC's position
      double adc_l = lhs.frenet_frame_path().front().l();
      if (adc_l < -1.0) {
        return lhs.path_label().find("right") != std::string::npos;
      } else if (adc_l > 1.0) {
        return lhs.path_label().find("left") != std::string::npos;
      }
    }
  }
  // If same length, both neighbor lane are forward,
  // then select the one that returns to in-lane earlier.
  static constexpr double kBackToSelfLaneComparisonTolerance = 20.0;
  int lhs_back_idx = GetBackToInLaneIndex(lhs.path_point_decision_guide());
  int rhs_back_idx = GetBackToInLaneIndex(rhs.path_point_decision_guide());
  double lhs_back_s = lhs.frenet_frame_path()[lhs_back_idx].s();
  double rhs_back_s = rhs.frenet_frame_path()[rhs_back_idx].s();
  if (std::fabs(lhs_back_s - rhs_back_s) > kBackToSelfLaneComparisonTolerance) {
    return lhs_back_idx < rhs_back_idx;
  }
  // If same length, both forward, back to inlane at same time,
  // select the left one to side-pass.
  bool lhs_on_leftlane = lhs.path_label().find("left") != std::string::npos;
  bool rhs_on_leftlane = rhs.path_label().find("left") != std::string::npos;
  if (lhs_on_leftlane != rhs_on_leftlane) {
    ADEBUG << "Select " << (lhs_on_leftlane ? "left" : "right") << " lane over "
           << (!lhs_on_leftlane ? "left" : "right") << " lane.";
    return lhs_on_leftlane;
  }
```
4. Update necessary info for lane-borrow decider's future uses.
在借道的路径上添加一些信息，包括：
- 前方障碍物的counter
- 当前车道可用的counter
- 当前的借道方向 

这里只是添加对应的status，具体如何使用还需要看后续的speed decider




