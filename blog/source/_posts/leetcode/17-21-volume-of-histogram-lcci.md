---
title: 面试题 17.21. 直方图的水量
date: 2021-04-02 10:15:42
mathjax: true
tags: leetcode
---



#### [面试题 17.21. 直方图的水量](https://leetcode-cn.com/problems/volume-of-histogram-lcci/) 

HARD

给定一个直方图(也称柱状图)，假设有人从上面源源不断地倒水，最后直方图能存多少水量?直方图的宽度为1。

![](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/10/22/rainwatertrap.png)

上面是由数组 [0,1,0,2,1,0,1,3,2,1,2,1] 表示的直方图，在这种情况下，可以接 6 个单位的水（蓝色部分表示水）。 感谢 Marcos 贡献此图。

示例:

>输入: [0,1,0,2,1,0,1,3,2,1,2,1]
>输出: 6

<!-- more -->

评论区老哥们的反应：

>打开一看，困难题（眉头一皱），定睛一看，接雨水，那没事了🤔

> 当你不会的时候 发现别人有三个Solution

> 这个题告诉我们，hard都不会，清明节还想出去浪？（你这个年纪你还睡的着觉，有点出息没有）

------

DP和双指针还有点点印象，但是DP想不起来了哈哈哈。。。

先把双指针写了，再看剩下的两个Solution：

定义左右两个指针left,right，从两侧向中间移动

定义左右指针移动过程中遇到的最大值leftMax和rightMax

哪个指针的对应的值value小，就移动哪个指针，

ans += 当前指针对应的最大值leftMax/rightMax-当前值的value

```c++
class Solution {
public:
    int trap(vector<int>& height) {
		int ans = 0;
		int left = 0, right = height.size() - 1;
		int leftMax = 0, rightMax = 0;
		while(left < right){
			leftMax = std::max(leftMax, height[left]);
            rightMax = std::max(rightMax, height[right]);
            if(height[left] < height[right]){
                ans += leftMax - height[left];
                ++left;
            }else{
                ans += rightMax - height[right];
                --right;
            }
		}
        return ans;
    }
};
```



DP:

左右依次扫射，然后选重叠区域

![fig1](https://assets.leetcode-cn.com/solution-static/jindian_17.21/1.png)

```c++
class Solution {
public:
    int trap(vector<int>& height) {
        int n = height.size();
        if (n == 0) {
            return 0;
        }
        vector<int> leftMax(n);
        leftMax[0] = height[0];
        for (int i = 1; i < n; ++i) {
            leftMax[i] = max(leftMax[i - 1], height[i]);
        }

        vector<int> rightMax(n);
        rightMax[n - 1] = height[n - 1];
        for (int i = n - 2; i >= 0; --i) {
            rightMax[i] = max(rightMax[i + 1], height[i]);
        }

        int ans = 0;
        for (int i = 0; i < n; ++i) {
            ans += min(leftMax[i], rightMax[i]) - height[i];
        }
        return ans;
    }
};
```

单调栈

栈底存左边界，栈顶存右边界，栈顶==栈底时，将(栈顶-栈底 - 1)加到结果里： 

```c++
class Solution{
public:
    int trap(vector<int>& height){
        int ans = 0;
        std::stack<int> stk;
        int n = height.size();
        for(int i = 0; i < n; ++i){
            while(!stk.empty() && height[i] > height[stk.top()]){
                int top = stk.top();
                stk.pop();
                if(stk.empty()){
                    break;
                }
                int left = stk.top();
                int currWidth = i - left - 1;
                int currHeight = min(heigth[left], height[i]) - height[top];
                ans += currWidth * currHeight;
            }
            stk.push(i);
        }
        return ans;
    }
};
```

