## 简介

这个repository创建的目的是通过学习DirectX来对图形学中相关理论和算法，以及GPU驱动和GPU设计架构有个更为全面体系的认识和了解。示例中有些来源于DXSDK，有些来源于网络，然后在此基础上进行加工或修改以达到探索某个特性或者了解某个现象涉及的原理的学习。这些基础的知识对于从事游戏开发，渲染引擎，GPU驱动或设计的相关技术人员或许有点帮助。
推荐几个相关的资源：
1. DXSDK中的案例和说明
2. NVSDK中的案例和说明
3. tiger book
4. 闫令祺大佬的games101课程
5. 个人博客： https://binean.top

## 00_DX11_SimpleInit
这个案例相当于学习C语言时候的hello world, 主要是创建窗口，创建DX Device, 初步了解DX的render pipline.

## 01_DX11_Init
对案例00的改造，使用面向对象的方式对00中的代码进行重构，本质上还是和00中的功能是一致的。

## 02_DX11_Triangle
在00的基础上画一个三角形，初步了解HLSL Shader的编写。

## 03_DX11_Cube
在02的基础上画一个正方体，了解HLSL Shader的编写，CB的使用，理解MVP变换。

## 04_DX_Transformation
在03的基础上再加一个正方体，理解坐标系的变换。

