## Monitor

从lcm通道中采集数据，实时动态解析proto类型的信号。

解析和渲染来自cyber_monitor，拆出了cyber模块，添加了lcm通道接收，修改了相关接口。

配置文件在channel_monitor_tool.pb.txt中，需要配置channel和proto的message_type


### 构建方式：

1. 安装ncurses5库 sudo apt update && sudo apt install libncurses5-dev -y

2. 将tools/channel_monitor中的#BUILD重命名为BUILD

3. 将tools/workspace/ncurses5目录中的#BUILD重命名为BUILD，#ncurses5.BUILD 重命名为 ncurses5.BUILD

4. 在autobrain/WORKSPACE文件中添加：`load("//tools/workspace/ncurses5:repository.bzl", "ncurses5_repository")` 和 `ncurses5_repository("ncurses5")`

5. Bazel build ... 位于/tools/channel_monitor目录



