---
title: HSM
date: 2021-04-06 14:50:05
tags: hsm
---
### Inner和InnerEntry

#### 问题

调试状态机时发现状态机状态反复切换，观察Debug信息：

<!-- more -->
```
HSM_2_hsm_info: Pop     : N2ab8planning22ChangeLaneStateMachine6ManualE
HSM_1_hsm_info: Sibling : N2ab8planning22ChangeLaneStateMachine4AutoE
HSM_1_hsm_info:  Inner   : N2ab8planning22ChangeLaneStateMachine11LaneKeepingE

HSM_2_hsm_info:  Pop     : N2ab8planning22ChangeLaneStateMachine11LaneKeepingE
HSM_1_hsm_info:  Sibling : N2ab8planning22ChangeLaneStateMachine13LeftIntendingE

HSM_2_hsm_info:  Pop     : N2ab8planning22ChangeLaneStateMachine13LeftIntendingE
HSM_1_hsm_info:  Inner   : N2ab8planning22ChangeLaneStateMachine11LaneKeepingE
HSM_2_hsm_info:  Pop     : N2ab8planning22ChangeLaneStateMachine11LaneKeepingE
HSM_1_hsm_info:  Sibling : N2ab8planning22ChangeLaneStateMachine13LeftIntendingE

```

按顺序进行如下切换：

1.退出**手动**状态

2.切换到**自动**状态

3.进入下一层的**LaneKeeping**状态

4.推出**LaneKeeping**状态

5.进入**LeftIntending**状态

6.推出**LeftIntending**状态

……

后面无限重复3-6步骤

------


#### 解决

本身状态机切换的GetTransition()函数正常

查阅[官方文档](https://github.com/amaiorano/hsm/wiki/Chapter-3.-The-H-in-HSM#inner-and-outer-states)后发现进入下一层状态机有两个template函数：
- InnerEntryTransition\<TargetState>
- InnerTransition\<TargetState> 

------

对于InnerEntryTransition来说：

>When a state returns an InnerEntryTransition\<TargetState>, the state machine will enter TargetState if no other inner state has been entered yet.

调用InnerEntryTransition时，如果下一层状态为空，会直接进入下一层的TargetState状态

>In other words, if a state at depth D on the state stack returns InnerEntryTransition\<TargetState>, as long as there is no state yet at depth D+1, TargetState will be created and pushed onto the stack. The next time the same state returns InnerEntryTransition\<TargetState>, because an inner state has already been pushed, this transition will be ignored.

也就是说假如目前在第D层，当D+1层有状态时，InnerEntryTransition会被忽略掉，D+1层没有状态时才能Push进状态。

------

对于InnerTransition来说：



>In the previous section, we covered the InnerEntryTransition, which is used to push an inner state only if there is no inner state yet on the stack. The more generalized InnerTransition, on the other hand, is used to force an inner state onto the state stack, regardless of what's on it.

InnerTransition一般用于强制指定下一层状态



>More specifically: when InnerTransition\<TargetState> is returned from GetTransition, the state machine makes sure that TargetState becomes (or remains) the inner state. If there is no inner state yet, TargetState gets pushed. If the inner state is already TargetState, nothing happens. If the inner state is not TargetState, the current inner state - along with all its inners - are popped off the stack (from innermost out), and TargetState is then pushed.

详细来说，调用InnerTransition时，会检查inner state是否是TargetState。

如果调用后的状态不是TargetState，会将下一层的状态pop出来，再push进TargetState。

------

#### 解决
对于上面的问题：

由于在LaneKeeping状态的GetTransition函数中会直接转到LeftIntending状态。

导致InnerTransition调用后，TargetState（LaneKeeping）和ReturnState（LeftIntending）不一致。接着会将LeftIntending推出，又重新推入LaneKeeping进入循环。

这里的问题是本应该使用InnerEntryTransition的场景，误用了InnerTransition函数。

------
另外看到wiki中的[best practices](https://github.com/amaiorano/hsm/wiki/Chapter-5.-Best-Practices#prefer-innerentry--sibling-to-inner)中也推荐InnerEntry+sibing的方式：

>Inner transitions are almost never the right type of transition to use. The only times it makes sense to use Inner transitions is when a state needs to force transitions to specific inner states immediately. Typically, these inner states map directly to an enum value or to a series of simple if-else conditions, for example:

```c++
struct Locomotion : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().PressedMove())
				return InnerTransition<Move>();
			else
				return InnerTransition<Stand>();
		}
	};
```