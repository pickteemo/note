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

# 2 id add to watch later
id_watchlater = []
id_watchlater.append("594534726")  # 小黑板

sleep_time = 2.0


def operate_favorite_by_mid(_mid):
    videos_list_g = user.get_videos_g(uid=_mid)
    for video_t in videos_list_g:
        # sleep
        time.sleep(sleep_time)
        # 跳过收藏的
        bvid = video_t["bvid"]
        if video.is_favoured(bvid=bvid, verify=verify):
            time.sleep(sleep_time)
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

        if _mid in id_watchlater:
            video.operate_favorite(add_media_ids="1191407326",
                                   bvid=bvid, verify=verify)  # 默认
            print("add watch later:\tauthor:" +
                  video_t["author"] + "\t video:" + video_t["title"])
        else:
            video.operate_favorite(add_media_ids="45360826",
                                   bvid=bvid, verify=verify)  # 默认
            print("add: \t author:" +
                  video_t["author"] + "\t video:" + video_t["title"])
        time.sleep(sleep_time)


verify = Verify(sessdata="55604017%2C1629537801%2Cc87fe*21",
                csrf="2a58a75e97b20a8ab1f9e840076fa93c")

followings_g = user.get_followings_g(uid=7647426, verify=verify)
relation = user.get_relation_info(uid=7647426, verify=verify)
following_num = relation["following"]
follow_list_id = []
count = 0
for following in followings_g:
    count = count + 1
    mid = following["mid"]
    operate_favorite_by_mid(mid)
    print("(" + str(count) + "/" + str(following_num) + ")", end='\r')
    time.sleep(sleep_time)
print("save over")
