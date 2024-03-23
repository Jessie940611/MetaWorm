# 目录

[编译](#编译)

[数据格式](#数据格式)

[启动项参数](#启动项参数)

[控制面板](#控制面板)

---
# 编译

## 生成项目

基本需求：

Cmake 2.8及以上

CUDA 10.0及以上（推荐10.0）

OptiX 7.0及以上（推荐7.0）

OpenGL

使用Cmake生成项目

Source选择 `root/dynamicneuronx/neuronXcore`

---

必须设置的参数有：

BIN2C

CUDA_HOST_COMPILER

CUDA_TOOLKIT_ROOT_DIR

OptiX_INCLUDE

其他参数默认即可

### windows下注意事项

生成器选择Visual Studio 15 2017, 平台选择X64

---
### linux下注意事项

CUDA_HOST_COMPILER 需要gcc版本不高于6，可以直接用nvcc

---
## 编译项目

---
### windows下

使用VS2017生成一次后，重新加载再生成一次即可，之后的修改不再需要重新生成。

---
### linux下

进入build目录，控制台输入make即可。

---
## 测试运行

推荐使用examples中的tubes数据进行测试。

测试结果可与`examples/windows/tubes.bat`或`examples/linux/tubes.sh`进行对比。

---
### windows下

VS的Command Arguments设置为 `-user tubes文件夹的路径`。

---
### linux下

在build目录下，控制台输入`./neuronXcore -user tubes文件夹的路径`。

---
如果二者的图像结构上一致，说明编译成功。

---
# 数据格式

当前，neuronXcore支持三种可视化模式。

核心的精细神经元可视化模式的数据是基于SDF的，类似于.swc文件的数据。

辅助的脑区可视化模式的数据是常用的三角面片数据。

试验性的大规模神经元结构可视化模式的数据是基于参数曲线的数据。

## 精细神经元可视化数据

数据包括三个部分：

记录节点位置属性的vertex文件

记录节点逻辑关系的indices文件

记录节点电压大小的rec_voltages文件

vertex文件是一种类似于swc的文件，每一行代表一个节点，数据从左至右依次代表：

`x坐标，y坐标，z坐标，半径r，区段编号i`

实际有效的数据是x, y, z, r。i的存在是历史原因。

indices文件记录的是每个节点之间的联系，每一行代表一组连接，数据包括：

`preNode，postNode`

rec_voltages文件是一个二进制数据文件，前四个数是固定的提示性数据，分别是节点数、时刻数、1和0。

之后的数据则为每个节点的电压数据，先按时刻排序，再按节点排序，即——时刻1：节点1、节点2、...、时刻2：节点1、节点2、...

---
## 脑区可视化模式数据

支持.obj文件，纹理映射。暂不支持多重材质。

---
## 大规模神经元结构可视化模式数据

试验性的模式，每一行代表神经元的一个区段（也可认为单纯就是一条曲线）。

基于三次多项式参数曲线的方程`f(t)=at^3+bt^2+ct+d`，数据从左到右依次是：

`ax, ay, az, bx, by, bz, cx, cy, cz, dx, dy, dz, ra, rb, rc, rd, red, green, blue`

其中ax代表x坐标的参数方程的系数a；

ra代表半径r的参数方程的系数a；

参数t的取值范围为0到1；

red，green，blue代表三通道颜色（0到1）。

---
# 启动项参数

启动项参数之间以空格分开。以下是常用启动项参数：

`-user PATH` 指定精细神经元可视化的数据文件。路径为数据所在文件夹的路径。

`-obj PATH` 指定脑区可视化的数据文件。路径为数据文件的路径。

`-curve PATH` 指定大规模神经元结构可视化的数据文件。路径为数据文件的路径。

`-mat PATH` 指定统一的材质。路径为材质文件的路径。

`-win INT INT` 指定窗口大小和分辨率，默认1280*720.

`-bg FLOAT` 指定背景亮度，范围从0到1.
 
---
以下是不常用的启动项参数

`--camera-fov INT` 指定fov（视场）大小，fov越大，图像看起来越近。

`--camera-focal-distance FLOAT` 相机近平面距离，控制透视投影的效果。

`-spp INT` 每像素采样数，控制可视化图像的精细程度，spp越高帧率越低。

`-accum INT` 控制自动播放电压序列时每个时刻的帧累积数。

`-max-accum INT` 最大帧累积数，控制某些场景下图像变化的平滑感，非正数代表无限。

`-measure` 运行60秒后截图退出。

`--enable-emissive` 开启精细神经元自发光。

部分无效的参数暂不列出。

---
# 控制面板

可视化运行过程中可以展开控制面板。控制面板如图所示：

![control pad](https://gitlab.com/tianyan/brainvissystem/dynamicneuronx/-/blob/dev_mahchine/neuronXcore/examples/images/controlpad.png "control pad")

控制面板分为五个部分，其中不少都与字面意思一致。

Camera Property涉及到一些调试功能。

---
Renderer Property涉及一些渲染的基础的参数调整。

其中：

path depth控制路径追踪的深度，越高渲染效果越好，推荐3。

spp控制每像素采样数，越高渲染效果越好，但帧率越低，推荐1。

shader mode控制着色模式，仅在精细神经元可视化模式中有效。0代表无光照（纯色），1代表直接光照，2代表全局光照。

max accum控制帧累积上限，非正数代表无限，当此项设为正数时，部分静态场景的变动不再重置帧的累积，变化更加平滑连贯。

---
Triangle Mesh Property涉及到脑区透明可视化的参数调整。

其中：

enable mesh transparency勾选后将开启透明模式，透明模式依照深度着色，由浅到深依次是绿-蓝-黄-红。

mesh transparency控制透明程度，即光线有多大概率穿透。

max transparency depth控制透明的层数，目前支持最大7层。

---
Voltage Property涉及精细神经元可视化的电压参数调整。

其中：

voltage animation勾选后，voltage sequence将以sequence step为步长进行移动，直到达到最大。

max accum设为正数时，拖动voltage sequence画面不再突变。

voltage threshold用于过滤低电压，低于该百分比的电压值将不再着色。

enable emissive开启后，节点将依照其电压在最大最小电压之间的位置来发光。

当接近minVoltage时，节点将变成黑色，接近maxVoltage时，节点将变成白色。

在这之间，inhibition对应蓝色，resting对应灰色，excitation对应红色，firing对应黄色。

关闭emissive后，节点依然会依照电压颜色对外界光照进行吸收和反射。

---
Light Property涉及光照的参数调整。

其中：

top color和bottom color控制背景颜色和环境光大小。

ground color目前没有效果。