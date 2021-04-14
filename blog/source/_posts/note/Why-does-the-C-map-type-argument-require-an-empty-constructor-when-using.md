---
title: 'Why does the C++ map type argument require an empty constructor when using []?'
date: 2021-04-13 10:23:24
tags:
---



#### STL Map容器中需要默认的构造函数 

搞一个Map<K,V>,其中V没有默认的构造函数

```c++
#include <map>

struct MyClass
{
    MyClass(int t);
};

int main() {
    std::map<int, MyClass> myMap;
    myMap[14] = MyClass(42);
}
```

编译时g++会报错：

> /usr/include/c++/4.3/bits/stl_map.h:419: error: no matching function for call to ‘MyClass()’

<!-- more -->

问题出在操作符[]

[]操作符会返回map内部对象的引用

在map为空时，map默认会向尾部插入一个默认构造函数的对象

所以这种情况，需要添加默认构造函数/重载[]/使用find、insert代替[]



This issue comes with operator[]. Quote from SGI documentation:

> `data_type& operator[](const key_type& k)` - Returns a reference to the object that is associated with a particular key. If the map does not already contain such an object, `operator[]` inserts the default object `data_type()`.

If you don't have default constructor you can use insert/find functions. Following example works fine:

```cpp
myMap.insert( std::map< int, MyClass >::value_type ( 1, MyClass(1) ) );
myMap.find( 1 )->second;
```





参考：

https://zhuanlan.zhihu.com/p/92635464

https://stackoverflow.com/questions/1935139/using-stdmapk-v-where-v-has-no-usable-default-constructor

https://stackoverflow.com/questions/695645/why-does-the-c-map-type-argument-require-an-empty-constructor-when-using