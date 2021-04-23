---
title: 368 最大整除合集
date: 2021-04-23 10:50:55
mathjax: true
tags: leetcode
---

#### [368. 最大整除子集](https://leetcode-cn.com/problems/largest-divisible-subset/)

<!-- more -->

先排序,再DP,最后倒序遍历:

dp[i]表示nums[i]的最大子集的大小

cpp:

```c++
class Solution {
public:
    vector<int> largestDivisibleSubset(vector<int>& nums) {
        int len = nums.size();
        std::sort(nums.begin() , nums.end());

        //step1 : DP
        vector<int> dp(len,1);
        int maxSize = 1, maxVal = nums[0];
        for(int i = 1; i<len ; ++i){
            for(int j = 0; j < i; ++j){
                if(nums[i] % nums[j] == 0){
                    dp[i] = max(dp[i],dp[j] + 1);
                }
            }
            if(dp[i] > maxSize){
                maxSize = dp[i];
                maxVal = nums[i];
            }
        }

        //step2 : Reverse traversal
        vector<int> res;
        if(maxSize == 1){
            res.push_back(nums[0]);
            return res;
        }

        for (int i = len - 1; i >= 0 && maxSize > 0; i--) {
            if (dp[i] == maxSize && maxVal % nums[i] == 0) {
                res.push_back(nums[i]);
                maxVal = nums[i];
                maxSize--;
            }
        }
        return res;
    }
};
```



