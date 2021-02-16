---
title: 'Path Decider'
date: 2021-02-15 09:46:56
tags: []
published: true
hideInList: false
feature: 
isTop: false
---
path decider负责给每个Obstacle打tag，供后续使用
<!-- more -->
file : modules/planning/tasks/deciders/path_decider/path_decider.cc


在不处于借道场景时（之前lane borrow decider中决定），对前方block的障碍物做停车处理，将停止点记录下来
```
    // - add STOP decision for blocking obstacles.
    if (obstacle->Id() == blocking_obstacle_id &&
        !injector_->planning_context()
             ->planning_status()
             .path_decider()
             .is_in_path_lane_borrow_scenario()) {
      // Add stop decision
      ADEBUG << "Blocking obstacle = " << blocking_obstacle_id;
      ObjectDecisionType object_decision;
      *object_decision.mutable_stop() = GenerateObjectStopDecision(*obstacle);
      path_decision->AddLongitudinalDecision("PathDecider/blocking_obstacle",
                                             obstacle->Id(), object_decision);
      continue;
    }
```

借道场景下：
首先排除距离参考线太远的一些Obstacle：
     - 不在s范围内的不考虑
     -  距离车道线（不是中心线）超过3m的

接下来如果横向有重叠部分，添加停止点信息，如果能够避让，添加避让信息

