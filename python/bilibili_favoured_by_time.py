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
start_time_u = end_time_u - (86400*1.5)

# 2 id add to watch later
id_watchlater = []
id_watchlater.append(594534726)  # 小黑板
id_watchlater.append(439004370)  # 一只撸狗
id_watchlater.append(7349)  # STN
id_watchlater.append(1565155)  # 是大腿
id_watchlater.append(10462362)  # 天天卡牌
id_watchlater.append(220746669)  # 嗨游君
id_watchlater.append(4641697)  # 最强联盟
id_watchlater.append(279991456)  # 靠谱电竞
id_watchlater.append(28870949)  # 陈小双






sleep_time = 7.0


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
            video.operate_favorite(add_media_ids=[1191407326],
                                   bvid=bvid, verify=verify)  # 
            print("add watch later:\tauthor:" +
                  video_t["author"] + "\t video:" + video_t["title"])
        else:
            video.operate_favorite(add_media_ids=[45360826],
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
print("save start")
for following in followings_g:
    time.sleep(sleep_time)
    count = count + 1
    mid = following["mid"]
    try:
        operate_favorite_by_mid(mid)
    except:
        continue
    else:
        print(time.asctime(time.localtime(time.time()))," (" + str(count) + "/" + str(following_num) + ") id:" + following["uname"] )

print("save over")
