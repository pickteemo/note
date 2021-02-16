
前面几个decider更偏向于make decision，通过外部环境选择自车的动作
接下来的目标是根据道路、障碍物以及前几步的decision-making建立数学模型来求解轨迹

视频：
<iframe src="//player.bilibili.com/player.html?aid=289090335&bvid=BV1qf4y1r7o5&cid=292510772&page=1" scrolling="no" border="0" frameborder="no" height="480" width="720" framespacing="0" allowfullscreen="true"> </iframe>
<!-- more -->

[![y1of1I.png](https://s3.ax1x.com/2021/02/04/y1of1I.png)](https://imgchr.com/i/y1of1I)

如上图所示，在frenet坐标系下，假设自车目前位置为s=0，l = 0
将车道线和障碍物转为自车两侧的边界线（图中红线），再均分得到中心线（灰线），
通过设计cost，将轨迹求解问题转为在boundary内求解l,dl,ddl,使cost最小的二次规划问题（通过求解器进行求解，图中蓝线为得到的解）。

Path Bound Decider的目的就是将车道线和障碍物转为上图中的边界

### 初始化
Task初始化，根据参考线建立frenet坐标系，计算自车在frenet坐标系中的位置(s,l,s',l'),同时计算自车到车道中心线的距离以及左右车道宽度。

接下来生成path bound，每个path bound都会在后续生成一条轨迹。
### Fall Back pathbound
首先生成一条默认的fall back pathbound，在正常求解轨迹无解或失败的情况下使用。
长度为(车速×8秒 , 100m)取大，间距为0.5m。
接下来遍历前方道路，依据下方规则生成左右两侧的boundary
如图：
[![y3FVx0.png](https://s3.ax1x.com/2021/02/04/y3FVx0.png)](https://imgchr.com/i/y3FVx0)

根据车道生成左右的lane_bound，从地图中获得
根据自车状态生成adc_bound，adc_bound = adc_l_to_lane_center_ + ADC_speed_buffer + adc_half_width + ADC_buffer

上式中的各项：
adc_l_to_lane_center_ - 自车
adc_half_width - 车宽
adc_buffer - 0.5
左侧当前s对应的bound取MAX(left_lane_bound,left_adc_bound),
右侧当前s对应的bound取MIN(right_lane_bound,right_adc_bound)（取远的）
ADCSpeedBuffer表示横向的瞬时位移？ 公式如下：
$$ADCSpeedBuffer = \frac{l'^2}{2\cdot kMaxLatAcc}$$ 
其中kMaxLatAcc = 1.5


### Regular Path Bound
接下来生成常规path boundary，根据当前场景(pull_over,lane_change,lane_borrow)选择对应的生成方法。以常规的LaneBorrowPath Bound为例：
#### Lane Borrow Path Bound
默认会生成一条不借道的bound，从path_decider_status中取出当前的借道状态，
有左换道就生成一条左换道的bound，
有右换道就生成一条右换道的bound，
都有就都生成。
```
  // Generate regular path boundaries.
  std::vector<LaneBorrowInfo> lane_borrow_info_list;
  lane_borrow_info_list.push_back(LaneBorrowInfo::NO_BORROW);

  if (reference_line_info->is_path_lane_borrow()) {
    const auto& path_decider_status =
        injector_->planning_context()->planning_status().path_decider();
    for (const auto& lane_borrow_direction :
         path_decider_status.decided_side_pass_direction()) {
      if (lane_borrow_direction == PathDeciderStatus::LEFT_BORROW) {
        lane_borrow_info_list.push_back(LaneBorrowInfo::LEFT_BORROW);
      } else if (lane_borrow_direction == PathDeciderStatus::RIGHT_BORROW) {
        lane_borrow_info_list.push_back(LaneBorrowInfo::RIGHT_BORROW);
      }
    }
  }
  ```
根据车道和自车状态生成boundary和上方fallback是一样的，adc_buffer调整为了0.1
```
  // 2. Decide a rough boundary based on lane info and ADC's position
  if (!GetBoundaryFromLanesAndADC(reference_line_info, lane_borrow_info, 0.1,
                                  path_bound, borrow_lane_type)) {
    const std::string msg =
        "Failed to decide a rough boundary based on "
        "road information.";
    AERROR << msg;
    return Status(ErrorCode::PLANNING_ERROR, msg);
  }
```
然后根据静态障碍物生成boundary,记录block位置之前的boundary。
场景示例如下：
[![y85a5Q.png](https://s3.ax1x.com/2021/02/05/y85a5Q.png)](https://imgchr.com/i/y85a5Q)
```
  PathBound temp_path_bound = *path_bound;
  if (!GetBoundaryFromStaticObstacles(reference_line_info.path_decision(),
                                      path_bound, blocking_obstacle_id)) {
    const std::string msg =
        "Failed to decide fine tune the boundaries after "
        "taking into consideration all static obstacles.";
    AERROR << msg;
    return Status(ErrorCode::PLANNING_ERROR, msg);
  }
  // Append some extra path bound points to avoid zero-length path data.
  int counter = 0;
  while (!blocking_obstacle_id->empty() &&
         path_bound->size() < temp_path_bound.size() &&
         counter < kNumExtraTailBoundPoint) {
    path_bound->push_back(temp_path_bound[path_bound->size()]);
    counter++;
  }
```
先来筛选障碍物，障碍物筛选规则如下，需要符合以下所有的条件，才加到obs_list中：
    - 不是虚拟障碍物
    - 不是可忽略的障碍物（其他decider中添加的ignore decision）
    - 静态障碍物或速度小于FLAGS_static_obstacle_speed_threshold（0.5m/s）
    - 在自车的前方
接下来将每个障碍物分解成两个ObstacleEdge，起点一个终点一个，记录下s，start_l,end_l,最后按s排序
[![y8szJf.png](https://s3.ax1x.com/2021/02/05/y8szJf.png)](https://imgchr.com/i/y8szJf)

对于上图场景中id为2的obstacle，sort后得到的内部数据结构为：
[![y85LIe.png](https://s3.ax1x.com/2021/02/05/y85LIe.png)](https://imgchr.com/i/y85LIe)
```
  for (const auto* obstacle : indexed_obstacles.Items()) {
    // Only focus on those within-scope obstacles.
    if (!IsWithinPathDeciderScopeObstacle(*obstacle)) {
      continue;
    }
    // Only focus on obstacles that are ahead of ADC.
    if (obstacle->PerceptionSLBoundary().end_s() < adc_frenet_s_) {
      continue;
    }
    // Decompose each obstacle's rectangle into two edges: one at
    // start_s; the other at end_s.
    const auto obstacle_sl = obstacle->PerceptionSLBoundary();
    sorted_obstacles.emplace_back(
        1, obstacle_sl.start_s() - FLAGS_obstacle_lon_start_buffer,
        obstacle_sl.start_l() - FLAGS_obstacle_lat_buffer,
        obstacle_sl.end_l() + FLAGS_obstacle_lat_buffer, obstacle->Id());
    sorted_obstacles.emplace_back(
        0, obstacle_sl.end_s() + FLAGS_obstacle_lon_end_buffer,
        obstacle_sl.start_l() - FLAGS_obstacle_lat_buffer,
        obstacle_sl.end_l() + FLAGS_obstacle_lat_buffer, obstacle->Id());
  }
```
```
  std::sort(sorted_obstacles.begin(), sorted_obstacles.end(),
            [](const ObstacleEdge& lhs, const ObstacleEdge& rhs) {
              if (std::get<1>(lhs) != std::get<1>(rhs)) {
                return std::get<1>(lhs) < std::get<1>(rhs);
              } else {
                return std::get<0>(lhs) > std::get<0>(rhs);
              }
            });
```
依次遍历按S排列的ObstacleEdge后，得到bound，就是下图中的红线
[![y1of1I.png](https://s3.ax1x.com/2021/02/04/y1of1I.png)](https://imgchr.com/i/y1of1I)
整体思路类似一条直线向前扫，扫到ObstacleEdge进入或退出就更新一下边界，

当左边界和右边界有重叠时，认为参考线被blocked，记录下位置。




### Reference
#### ReferenceLine::ToFrenetFrame
需要将起点转为frenet坐标系
cartesian to frenet reference :
[Frenet坐标系与Cartesian坐标系互转](https://blog.csdn.net/u013468614/article/details/108748016)
[Apollo项目坐标系研究](https://blog.csdn.net/davidhopper/article/details/79162385) 