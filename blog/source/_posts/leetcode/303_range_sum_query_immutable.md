---
title: 303. 区域和检索 - 数组不可变
date: 2021-03-29 09:38:40
mathjax: true
tags: leetcode
---

### [303. 区域和检索 - 数组不可变](https://leetcode-cn.com/problems/range-sum-query-immutable/)

> 给定一个整数数组  nums，求出数组从索引 i 到 j（i ≤ j）范围内元素的总和，包含 i、j 两点。
>
> 实现 NumArray 类：
>
> NumArray(int[] nums) 使用数组 nums 初始化对象
> int sumRange(int i, int j) 返回数组 nums 从索引 i 到 j（i ≤ j）范围内元素的总和，包含 i、j 两点（也就是 sum(nums[i], nums[i + 1], ... , nums[j])）
> 

<!-- more -->

>
>
> 示例：
>
> 输入：
> ["NumArray", "sumRange", "sumRange", "sumRange"]
> [[[-2, 0, 3, -5, 2, -1]], [0, 2], [2, 5], [0, 5]]
> 输出：
> [null, 1, -1, -3]
>
> 解释：
> NumArray numArray = new NumArray([-2, 0, 3, -5, 2, -1]);
> numArray.sumRange(0, 2); // return 1 ((-2) + 0 + 3)
> numArray.sumRange(2, 5); // return -1 (3 + (-5) + 2 + (-1)) 
> numArray.sumRange(0, 5); // return -3 ((-2) + 0 + 3 + (-5) + 2 + (-1))
>
>
> 提示：
>
> 0 <= nums.length <= 104
> -105 <= nums[i] <= 105
> 0 <= i <= j < nums.length
> 最多调用 104 次 sumRange 方法
>
> 来源：力扣（LeetCode）
> 链接：https://leetcode-cn.com/problems/range-sum-query-immutable
> 著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。

暴力法：

```c++
class NumArray {
public:
	NumArray(vector<int>& nums) {
        m_num = nums; 
    }
    int sumRange(int i, int j) {
        int res = 0;
        for(size_t index = i ;index <= j ; ++index){
            res += m_num[index];
        }
        return res;
    }
    std::vector<int> m_num;
};
```

考虑到sumRange可能会多次调用，可以提前把和先算出来

```c++
class NumArray {
public:
    NumArray(vector<int>& nums) {
        m_num = nums;
        int sum = 0;
        for(const int  &num : nums){
            sum += num;
            m_sum.push_back(sum);
        }
    }
    
    int sumRange(int i, int j) {
        return m_sum[j] - m_sum[i] + m_num[i];
    }
    std::vector<int> m_num;
    std::vector<int> m_sum;        
};

```

= = 应该不需要存num的

```python
class NumArray:

    def __init__(self, nums: List[int]):
        self.sums = [0]
        _sums = self.sums

        for num in nums:
            _sums.append(_sums[-1] + num)

    def sumRange(self, i: int, j: int) -> int:
        _sums = self.sums
        return _sums[j + 1] - _sums[i]
```

