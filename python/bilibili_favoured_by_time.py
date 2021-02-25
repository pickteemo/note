from bilibili_api import Verify
from bilibili_api import user
from bilibili_api import video

import time
import datetime

# 1、datetime转unix时间戳
now_time = datetime.datetime.now()
#start_time = datetime.date(2021,2,21)
#end_time = datetime.date(2021,2,23)
#start_time_u = int(time.mktime(start_time.timetuple()))
end_time_u = int(time.mktime(now_time.timetuple()))
start_time_u = end_time_u - (86400 * 2)


def operate_favorite_by_mid(_mid):
    videos_list_g = user.get_videos_g(uid=_mid)
    for video_t in videos_list_g:
        # sleep
        time.sleep(5.0) 
        # 跳过收藏的
        bvid = video_t["bvid"]
        if video.is_favoured(bvid=bvid, verify=verify):
            # sleep
            time.sleep(5.0)
            #print("favoured,break.\tauthor:" +
            #     video_t["author"] + "\t video:" + video_t["title"])
            break

        # 跳过太新的
        create_time = video_t["created"]
        if create_time > end_time_u:
#             print("too new,skip.\tauthor:" +
#                   video_t["author"] + "\t video:" + video_t["title"])
            continue
        elif create_time < start_time_u:
#             print("too OLD,break.\tauthor:" +
#                   video_t["author"] + "\t video:" + video_t["title"])
            break

        # favoured
        print("set favoured.\tauthor:" +
              video_t["author"] + "\t video:" + video_t["title"])
        video.operate_favorite(add_media_ids="45360826",
                               bvid=bvid, verify=verify)
        time.sleep(5.0)
 


verify = Verify(sessdata="55604017%2C1629537801%2Cc87fe*21",
                csrf="2a58a75e97b20a8ab1f9e840076fa93c")

followings_g = user.get_followings_g(uid=7647426, verify=verify)
follow_list_id = []
for following in followings_g:
    mid = following["mid"]
    operate_favorite_by_mid(mid)
    time.sleep(5.0)
