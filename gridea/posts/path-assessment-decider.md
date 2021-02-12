---
title: 'Path Assessment Decider'
date: 2021-02-08 14:31:16
tags: []
published: true
hideInList: false
feature: 
isTop: false
---
path optimizer根据boundaries生成了很多条空间上的path
在进行速度规划之前，需要先验证生成的path的合理性
<!-- more -->

file:modules/planning/tasks/deciders/path_assessment_decider/path_assessment_decider.cc

代码的注释还是比较清晰的，主要分成四部分：
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
#### Is valid regulat path
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
挑选最好的path




to do 
std bind    std placeholders



