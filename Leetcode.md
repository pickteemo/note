## 数组

### 561 数组拆分 - Easy

> 给定长度为 2n 的整数数组 nums ，你的任务是将这些数分成 n 对, 例如 (a1, b1), (a2, b2), ..., (an, bn) ，使得从 1 到 n 的 min(ai, bi) 总和最大。

返回该 最大总和 。
这是比各个语言的排序速度吧。。。
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



### 697.数组的度 - Easy

> 给定一个非空且只包含非负数的整数数组 nums，数组的度的定义是指数组里任一元素出现频数的最大值。
>
> 你的任务是在 nums 中找到与 nums 拥有相同大小的度的最短连续子数组，返回其长度。

中文题目挺绕的，看了眼英文版

>Given a non-empty array of non-negative integers `nums`, the **degree** of this array is defined as the maximum frequency of any one of its elements.
>
>Your task is to find the smallest possible length of a (contiguous) subarray of `nums`, that has the same degree as `nums`.

hahaha也挺绕的，解决起来还是比较easy

1. 记录每个元素出现的次数 - degree
2. 记录每个元素首次出现的位置
3. 遍历，返回degree最大的元素的长度

cpp

```c++
class Solution {
public:
    int findShortestSubArray(vector<int>& nums) {\
        std::unordered_map<int,int> count , begin;
        int res = 0,degree = 0;
        for (int i = 0;i < nums.size() ; ++i){
            if(begin.count(nums[i]) == 0){
                begin[nums[i]] = i;
            }
            if(count[nums[i]]++ > degree){
                degree = count[nums[i]];
                res = i - begin[nums[i]] + 1;
            }else if(count[nums[i]] == degree){
                res = std::min(res , i - begin[nums[i]] + 1);
            }
        }
        return res;
    }
};
```

python

```python
class Solution:
    def findShortestSubArray(self, nums: List[int]) -> int:
        begin , count = {},{}
        res , degree = 0,0
        for i, a in  enumerate(nums):
            begin.setdefault(a,i)
            count[a] = count.get(a,0)+1
            if count[a] > degree:
                degree = count[a]
                res = i - begin[a] + 1
            elif count[a] == degree:
                res = min(res,i - begin[a] + 1)
        return res
            
```



