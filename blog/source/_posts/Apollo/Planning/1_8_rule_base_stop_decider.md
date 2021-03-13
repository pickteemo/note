---
title: 1.8 Rule Base Stop Decider
date: 2021-03-13 19:11:43
tags: Apollo
---

设置停止点

<!-- more -->

### 设置停止点

设置这些场景中的停止点：

靠边停车

紧急换道

参考线终点

```c++
  // 1. Rule_based stop for side pass onto reverse lane
  StopOnSidePass(frame, reference_line_info);

  // 2. Rule_based stop for urgent lane change
  if (FLAGS_enable_lane_change_urgency_checking) {
    CheckLaneChangeUrgency(frame);
  }

  // 3. Rule_based stop at path end position
  AddPathEndStop(frame, reference_line_info);

```

