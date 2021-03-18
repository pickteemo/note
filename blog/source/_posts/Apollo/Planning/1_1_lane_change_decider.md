---
title: 1.1 Lane Change Decider
date: 2021-03-07 19:11:43
tags: Apollo
---

> Lane Change Decider

<!-- more -->



File Path: modules/planning/tasks/deciders/lane_change_decider/lane_change_decider.cc

首先取出上一刻的换道状态prev_status
```
auto* prev_status = injector_->planning_context()
                          ->mutable_planning_status()
                          ->mutable_change_lane();
```
prev_status是定义在proto中的结构体(modules/planning/proto/planning_status.proto)
包括：
Status - IN_CHANGE_LANE、CHANGE_LANE_FAILED 、CHANGE_LANE_FINISHED
path_id、timestamp

exist_lane_change_start_position - 换道起点flag
lane_change_start_postion - 换道起点

last_succeed_timestamp

is_current_opt_succeed - path和speed是否规划成功
is_clear_to_change_lane - 周围环境是否clear
```
message ChangeLaneStatus {
  enum Status {
    IN_CHANGE_LANE = 1;        // during change lane state
    CHANGE_LANE_FAILED = 2;    // change lane failed
    CHANGE_LANE_FINISHED = 3;  // change lane finished
  }
  optional Status status = 1;
  // the id of the route segment that the vehicle is driving on
  optional string path_id = 2;
  // the time stamp when the state started.
  optional double timestamp = 3;
  // the starting position only after which lane-change can happen.
  optional bool exist_lane_change_start_position = 4 [default = false];
  optional apollo.common.Point3D lane_change_start_position = 5;
  // the last time stamp when the lane-change planning succeed.
  optional double last_succeed_timestamp = 6;
  // if the current path and speed planning on the lane-change
  // reference-line succeed.
  optional bool is_current_opt_succeed = 7 [default = false];
  // denotes if the surrounding area is clear for ego vehicle to
  // change lane at this moment.
  optional bool is_clear_to_change_lane = 8 [default = false];
}
```
将初始状态初始化为CHANGE_LANE_FINISHED：
```
  if (!prev_status->has_status()) {
    UpdateStatus(now, ChangeLaneStatus::CHANGE_LANE_FINISHED,
                 GetCurrentPathId(*reference_line_info));
    prev_status->set_last_succeed_timestamp(now);
    return Status::OK();
  }
```
在有多条参考线的情况下认为可以换道
`bool has_change_lane = reference_line_info->size() > 1;`
然后进行换道状态的判断，逻辑框图大致如下：
![](https://pickteemo.github.io//post-images/1612340002243.jpg)

更新状态后Lane_Change_Decider就跑完了。


相关内容：

1. 这里有个疑问，reference_line_info > 1时就直接进入IN_CHANGE_LANE了，
也就是说只有在需要换道时，routing才会给出多条参考线吗？

2. 在lane_change_decider的配置中有一项reckless_change_line,默认关闭，打开后直接执行优先换道然后退出task，和正常的相比少了failed_freeze_time和success_freeze_time.

3. PrioritizeChangeLane（）函数，传True表示优先处理换道path，False表示优先处理当前path。
具体做法为：遍历Reference_line_inf（std::list type），找到第一个符合要求的path（True- ischangelane || False - is not changelane），将它插回Reference_line_info的头部
代码：
```
auto iter = reference_line_info->begin();
  while (iter != reference_line_info->end()) {
    ADEBUG << "iter->IsChangeLanePath(): " << iter->IsChangeLanePath();
    /* is_prioritize_change_lane == true: prioritize change_lane_reference_line
       is_prioritize_change_lane == false: prioritize
       non_change_lane_reference_line */
    if ((is_prioritize_change_lane && iter->IsChangeLanePath()) ||
        (!is_prioritize_change_lane && !iter->IsChangeLanePath())) {
      ADEBUG << "is_prioritize_change_lane: " << is_prioritize_change_lane;
      ADEBUG << "iter->IsChangeLanePath(): " << iter->IsChangeLanePath();
      break;
    }
    ++iter;
  }
  reference_line_info->splice(reference_line_info->begin(),
                              *reference_line_info, iter);
```
[链接：std::list<T>::splice on cppreference](https://zh.cppreference.com/w/cpp/container/list/splice )
