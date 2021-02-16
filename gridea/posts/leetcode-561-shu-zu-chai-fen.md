---
title: 'LeetCode 561 数组拆分'
date: 2021-02-16 09:59:53
tags: []
published: true
hideInList: false
feature: 
isTop: false
---
https://leetcode-cn.com/problems/array-partition-i/
数组

给定长度为 2n 的整数数组 nums ，你的任务是将这些数分成 n 对, 例如 (a1, b1), (a2, b2), ..., (an, bn) ，使得从 1 到 n 的 min(ai, bi) 总和最大。

返回该 最大总和 。
<!-- more -->
😅评论说的好,这是比各个语言的排序速度吧
最大总和 - 排序后返回奇数项之和
最小总和 - 排序后返回前n项
```
class Solution {
public:
    int arrayPairSum(vector<int>& nums) {
        std::sort(nums.begin(),num.end());
        int res = 0;
        for(int i = 0 ; i <nums.size() ; i = i+2){
            res += nums[i];
        }
        return res;
    }
};
```
```
class Solution:
    def arrayPairSum(self, nums: List[int]) -> int:
        nums.sort()
        return sum(nums[::2])
```
