---
title: Paper
date: 2021-04-08 15:55:11
tags: paper
---
Paper 
<!-- more -->

## Prediction

------

### A Kinematic Model for Trajectory Prediction in General Highway Scenarios

Abstract:Highway driving invariably combines high speeds with the need to interact closely with other drivers. Prediction methods enable autonomous vehicles (AVs) to anticipate drivers’ future trajectories and plan accordingly. Kinematic methods for prediction have traditionally ignored the presence of other drivers, or made predictions only for a limited set of scenarios.

Data-driven approaches fill this gap by learning from large datasets to predict trajectories in general scenarios. While they achieve high accuracy, they also lose the interpretability and tools for model validation enjoyed by kinematic methods. This letter proposes a novel kinematic model to describe car-following and lane change behavior, and extends it to predict trajectories in general scenarios. Experiments on highway datasets under varied sensing conditions demonstrate that the proposed method outperforms state-of-the-art methods.

title:高速公路场景下基于运动学模型的轨迹预测方法

abstract:高速公路驾驶包括高速与与其他驾驶员的交互两个特点。预测模块可以使自动驾驶汽车（AVs）预测其他车辆的的未来轨迹并做出相应的规划。传统方法使用运动学方式进行预测，会忽略掉其他车辆的影响，并且针对的场景也比较有限。

基于数据驱动，通过学习大型数据集来预测的轨迹的方法，填补了这一空白。这种方法虽然准确性高，但也失去了运动学方法的解释性和模型验证工具。 这篇论文提出了一种新颖的运动学模型来描述汽车跟随和变道行为，并将其扩展为在一般情况下预测轨迹。 在动态环境下对高速公路数据集进行的实验验证说明，该方法优于目前的相关方法。

pdf：[2103.16673](https://arxiv.org/pdf/2103.16673.pdf)

------


## Decision Making

------

### Uncovering Interpretable Internal States of Merging Tasks at Highway On-Ramps for Autonomous Driving Decision-Making
<br/>

title: 在高速公路匝道上发现可合并的任务的可解释内部状态，以进行自动驾驶决策

pdf:[2102.07530](https://arxiv.org/pdf/2102.07530.pdf)

abstract:Abstract—Humans make daily-routine decisions based on their internal states in intricate interaction scenarios. This paper presents a probabilistically reconstructive learning approach to identify the internal states of multi-vehicle sequential interactions when merging at highway on-ramps. We treated the merging task’s sequential decision as a dynamic, stochastic process and then integrated the internal states into an HMM-GMR model, a probabilistic combination of an extended Gaussian mixture regression (GMR) and hidden Markov models (HMM). We also developed a variant expectation-maximum (EM) algorithm to estimate the model parameters and verified them based on a realworld data set. Experimental results reveal that the interactive merge procedure at highway on-ramps can be semantically described by three interpretable internal states. This finding provides a basis for autonomous vehicles to develop a model-based decision-making algorithm in a partially observable environment.

Note to Practitioners—Model-based learning approaches have obtained increasing attention in decision-maker design due to their stability and interpretability. This paper was built upon the two facts: (1) Intelligent agents can only receive partially observable environment information directly through their equipped sensors in the real-world; (2) Humans mainly utilize the internal states and associated dynamics inferred from observations to make proper decisions in complex environments. Similarly, autonomous vehicles (AVs) need to understand, infer, anticipate, and exploit the internal states of dynamic environments. Applying probabilistic decision-making models to AVs requires updating the internal states’ beliefs and associated dynamics after getting new observations. The designed and verified emission model in the HMM-GMR can provide a modifiable functional module for online update of the associated internal states. Experimental results based on the real-world driving dataset demonstrates that the internal states extracted using HMM-GMR can represent the dynamic decision-making process semantically and make an accurate prediction.

abstract:人类在复杂的交互场景中根据其内部状态做出日常决策。 本文提出了一种概率重构学习方法，以识别在高速公路匝道上合并时多车辆顺序交互的内部状态。 我们将合并任务的顺序决策视为动态的随机过程，然后将内部状态集成到HMM-GMR模型中，该模型是扩展高斯混合回归（GMR）和隐马尔可夫模型（HMM）的概率组合。 我们还开发了一种变异期望最大值（EM）算法来估计模型参数，并根据实际数据集对它们进行验证。 实验结果表明，高速公路坡道上的交互式合并过程可以通过三种可解释的内部状态进行语义描述。 这一发现为自动驾驶汽车在部分可观察的环境中开发基于模型的决策算法提供了基础。

给从业者的注意-由于基于模型的学习方法的稳定性和可解释性，因此在决策者设计中受到越来越多的关注。 本文基于以下两个事实：（1）智能代理只能通过其在现实世界中配备的传感器直接接收部分可观察到的环境信息；  （2）人类主要利用从观察中推断出的内部状态和相关动力学来在复杂的环境中做出正确的决定。 同样，自动驾驶汽车（AV）需要了解，推断，预测和利用动态环境的内部状态。 将概率决策模型应用于视音频，需要在获得新的观察结果后更新内部状态的信念和相关动态。  HMM-GMR中经过设计和验证的排放模型可以为在线更新相关内部状态提供可修改的功能模块。 基于现实世界驾驶数据集的实验结果表明，使用HMM-GMR提取的内部状态可以在语义上表示动态决策过程并做出准确的预测。

------

### Corner Case Generation and Analysis for Safety Assessment of Autonomous Vehicles

自动驾驶汽车安全评估的案例分析与分析

ABSTRACT：Testing and evaluation is a crucial step in the development and deployment of Connected and Automated Vehicles (CAVs). To comprehensively evaluate the performance of CAVs, it is of necessity to test the CAVs in safety-critical scenarios, which rarely happen in naturalistic driving environment. Therefore, how to purposely and systematically generate these corner cases becomes an important problem. Most existing studies focus on generating adversarial examples for perception systems of CAVs, whereas limited efforts have been put on the decision-making systems, which is the highlight of this paper. As the CAVs need to interact with numerous background vehicles (BVs) for a long duration, variables that define the corner cases are usually high dimensional, which makes the generation a challenging problem. In this paper, a unified framework is proposed to generate corner cases for the decision-making systems. To address the challenge brought by high dimensionality, the driving environment is formulated based on Markov Decision Process, and the deep reinforcement learning techniques are applied to learn the behavior policy of BVs.

With the learned policy, BVs will behave and interact with the CAVs more aggressively, resulting in more corner cases. To further analyze the generated corner cases, the techniques of feature extraction and clustering are utilized. By selecting representative cases of each cluster and outliers, the valuable corner cases can be identified from all generated corner cases. Simulation results of a highway driving environment show that the proposed methods can effectively generate and identify the valuable corner cases.

摘要测试和评估是互联汽车和无人驾驶汽车（CAV）的开发和部署中的关键步骤。 为了全面评估CAV的性能，有必要在对安全至关重要的情况下对CAV进行测试，而这种情况在自然驾驶环境中很少发生。 因此，如何有目的地和系统地生成这些极端情况成为一个重要的问题。 现有的大多数研究都集中于为CAV的感知系统生成对抗性示例，而对决策系统的投入有限，这是本文的重点。 由于CAV需要长时间与众多背景车辆（BV）进行交互，因此定义拐角情况的变量通常是高维的，这使发电成为一个具有挑战性的问题。 本文提出了一个统一的框架来为决策系统生成极端案例。 为了应对高维带来的挑战，基于马尔可夫决策过程制定了驾驶环境，并运用深度强化学习技术来学习BV的行为策略。

借助学习到的策略，BV会更加积极地与CAV互动并与之互动，从而导致更多的极端情况。 为了进一步分析生成的极端情况，利用了特征提取和聚类技术。 通过选择每个聚类和离群值的代表性案例，可以从所有生成的极端案例中识别出有价值的极端案例。 公路驾驶环境的仿真结果表明，所提出的方法可以有效地生成和识别有价值的拐角案例。

pdf:[pdf](https://arxiv.org/pdf/2102.03483.pdf)


------

## Planning

### An Autonomous Driving Framework for Long-term Decision-making and Short-term Trajectory Planning on Frenet Space
<br/>

title: Frenet空间上长周期决策与实时规划框架

pdf: [pdf](https://arxiv.org/pdf/2011.13099)

code: [github](https://github.com/MajidMoghadam2006/frenet-trajectory-planning-framework)

image:
![](https://github.com/MajidMoghadam2006/frenet-trajectory-planning-framework/raw/master/case_1_agile_2d.gif)
![ima1](https://github.com/MajidMoghadam2006/frenet-trajectory-planning-framework/raw/master/case_1_contRL_2d.gif)

Abstract:In this paper, we present a hierarchical framework for decision-making and planning on highway driving tasks. We utilized intelligent driving models (IDM and MOBIL) to generate long-term decisions based on the traffic situation flowing around the ego. The decisions both maximize ego performance while respecting other vehicles’ objectives. Short-term trajectory optimization is performed on the Frenet space to make the calculations invariant to the road’s three-dimensional curvatures. A novel obstacle avoidance approach is introduced on the Frenet frame for the moving obstacles. The optimization explores the driving corridors to generate spatiotemporal polynomial trajectories to navigate through the traffic safely and obey the BP commands.

The framework also introduces a heuristic supervisor that identifies unexpected situations and recalculates each module in case of a potential emergency. Experiments in CARLA simulation have shown the potential and the scalability of the framework in implementing various driving styles that match human behavior.

摘要:在本文中，我们提出了应用于高速公路的决策和规划的分层框架。 我们利用智能驾驶模型（IDM和MOBIL）根据周围的交通状况在长周期层面上进行决策。这些决策指令在考虑其他车辆动作的同时，最大限度地提高了自我表现。 在Frenet空间上进行了短期轨迹优化，以使计算不依赖于道路的三维曲率。 在Frenet坐标系基础上引入了一种新颖的避障方法来应对动态障碍物。 根据可行驶区域进行优化来生成包括时间维度的多项式轨迹，来保证行车安全并遵守BP命令。

该框架还引入了启发式管理器，该管理器可识别意外情况并在潜在紧急情况下重新计算每个模块。  CARLA模拟中的实验表明，该框架在实现与人类行为相匹配的各种驾驶方式中具有潜力和可扩展性。

