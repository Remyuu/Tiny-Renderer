# 简易的光栅化渲染器

本文是一个完整的图形学入门实践课程，目前还在更新中，GitHub已开源。理论上本文项目需要20-30个小时完成。不知道为啥我的网站统计字数也有问题。

主要内容是完全手撸一个光栅化渲染器。本文会从头复习图形学以及C++的相关知识，包括从零构造向量模版库、光栅化原理解释、图形学相关基础算法解释等等内容。

另外原作者的的透视矩阵部分是经过一定程度的简化的，与虎书等正统做法不同。我会先按照原文ssloy老师的思想表达关键内容，最后按照我的想法完善本文。并且，原项目中的数学向量矩阵库写得不是很好，我专门开了一章一步步重构这个库。

> 原项目链接：https://github.com/ssloy/tinyrenderer
>
> 本项目链接：https://github.com/Remyuu/Tiny-Renderer

<!--more-->

[TOC]

## 0 简单的开始

五星上将曾经说过，懂的越少，懂的越多。我接下来将提供一个tgaimage的模块，你说要不要仔细研究研究？我的评价是不需要，如学。毕竟懂的越多，懂的越少。

在这里提供一个最基础的[框架🔗](https://github.com/Remyuu/Tiny-Renderer/tree/1.1_Bresenham’s_Line_Drawing_Algorithm)，他只包含了tgaimage模块。该模块主要用于生成.TGA文件。以下是一个最基本的框架代码：

```c++
// main.cpp
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    // TODO: Draw sth
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
```

上面代码会创建一个100*100的image图像，并且以tga的格式保存在硬盘中。我们在TODO中添加代码：

```c++
image.set(1, 1, red);
```

代码作用是在(1, 1)的位置将像素设置为红色。output.tga的图像大概如下所示：

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230830170319909.png" alt="0.0" style="zoom: 400%;" />

## 1.1 画线

这一章节的目标是画线。具体而言是制作一个函数，传入两个点，在屏幕上绘制线段。

### 第一关：实现画线

给定空间中的两个点，在两点(x0, y0)(x1, y1)之间绘制线段。

最简单的代码如下：

```c++
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
    for (float t=0.; t<1.; t+=.01) { 
        int x = x0 + (x1-x0)*t; 
        int y = y0 + (y1-y0)*t; 
        image.set(x, y, color); 
    } 
}
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230830171306158.png" alt="image-20230830171306158" />

### 第二关：发现BUG

上面代码中的.01其实是错误的。不同的分辨率对应的绘制步长肯定不一样，太大的步长会导致：

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230830171714587.png" alt="image-20230830171714587" />

所以我们的逻辑应该是：需要画多少像素点就循环Draw多少次。最简单的想法可能是绘制x1-x0个像素或者是y1-y0个像素：

```c++
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        image.set(x, y, color);
    }
}
```

上面代码是最简单的插值计算。但是这个算法是错误的。画三条线：

```c++
line(13, 20, 80, 40, image, white); 
line(20, 13, 40, 80, image, red); 
line(80, 40, 13, 20, image, blue);
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230830172534739.png" alt="image-20230830172534739" />

白色线看起来非常好，红色线看起来断断续续的，蓝色线直接看不见了。于是总结出以下两个问题：

1. 理论上说白色线和蓝色线应该是同一条线，只是起点与终点不同
2. 太“陡峭”的线效果不对

接下来就解决这个两个问题。

> 此处“陡峭”的意思是(y1-y0)>(x1-x0)
>
> 下文“平缓”的意思是(y1-y0)<(x1-x0)

### 第三关：解决BUG

为了解决起点终点顺序不同导致的问题，只需要在算法开始时判断两点x分量的大小：

```c++
if (x0>x1) {
    std::swap(x0, x1); 
    std::swap(y0, y1); 
}
```

为了画出没有空隙的“陡峭”线，只需要将“陡峭”的线变成“平缓”的线。最终的代码：

```c++
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    if(std::abs(x0-x1)<std::abs(y0-y1)) { // “陡峭”线
        if (y0 > y1) { // 确保从下到上画画
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        for (int y = y0; y <= y1; y++) {
            float t = (y - y0) / (float) (y1 - y0);
            int x = x0 * (1. - t) + x1 * t;
            image.set(x, y, color);
        }
    }
    else { // “平缓”线
        if (x0 > x1) { // 确保从左到右画画
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        for (int x = x0; x <= x1; x++) {
            float t = (x - x0) / (float) (x1 - x0);
            int y = y0 * (1. - t) + y1 * t;
            image.set(x, y, color);
        }
    }
}
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230830174932047.png" alt="image-20230830174932047" />

如果你想测试你自己的代码是否正确，可以尝试绘制出以下的线段：

```c++
line(25,25,50,100,image,blue);
line(25,25,50,-50,image,blue);
line(25,25,0,100,image,blue);
line(25,25,0,-50,image,blue);

line(25,25,50,50,image,red);
line(25,25,50,0,image,red);
line(25,25,0,0,image,red);
line(25,25,0,50,image,red);

line(25,25,50,36,image,white);
line(25,25,50,16,image,white);
line(25,25,0,16,image,white);
line(25,25,0,36,image,white);
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230831145010708.png" alt="image-20230831145010708" style="zoom:200%;" />

### 第四关：优化前言

目前为止，代码运行得非常顺利，并且具备良好的可读性与精简度。但是，画线作为渲染器最基础的操作，我们需要确保其足够高效。

性能优化是一个非常复杂且系统的问题。在优化之前需要明确优化的平台和硬件。在GPU上优化和CPU上优化是完全不同的。我的CPU是Apple Silicon M1 pro，我尝试绘制了9,000,000条线段。

发现在line()函数内，`image.set();`函数占用时间比率是38.25%，构建TGAColor对象是19.75%，14%左右的时间花在内存拷贝上，剩下的25%左右的时间花费则是我们需要优化的部分。下面的内容我将以运行时间作为测试指标。

### 第五关：Bresenham's 优化

我们注意到，for循环中的除法操作是不变的，因此我们可以将除法放到for循环外面。并且通过斜率估计每向前走一步，另一个轴的增量error。dError是一个误差积累，一旦误差积累大于半个像素（0.5），就对像素进行一次修正。

```c++
// 第一次优化的代码
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    if(std::abs(x0-x1)<std::abs(y0-y1)) { // “陡峭”线
        if (y0>y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        float dError = std::abs(dx / float(dy));
        float error = 0;
        int x = x0;
        for (int y = y0; y <= y1; y++) {
            image.set(x, y, color);
            error += dError;
            if (error>.5) {
                x += (x1>x0?1:-1);
                error -= 1.;
            }
        }
    }else { // “平缓”线
        if (x0>x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        float dError = std::abs(dy / float(dx));
        float error = 0;
        int y = y0;
        for (int x = x0; x <= x1; x++) {
            image.set(x, y, color);
            error += dError;
            if (error>.5) {
                y += (y1>y0?1:-1);
                error -= 1.;
            }
        }
    }
}
```

> 没有优化用时：2.98s
>
> 第一次优化用时：2.96s

### 第六关：注意流水线预测

在很多教程当中，为了方便修改，会用一些trick将“陡峭”的线和“平缓”的线的for循环代码整合到一起。即先将“陡峭”线两点的xy互换，最后再image.set()的时候再换回来。

```c++
// 逆向优化的代码
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    float dError = std::abs(dy/float(dx));
    float error = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error += dError;
        if (error>.5) {
            y += (y1>y0?1:-1);
            error -= 1.;
        }
    }
}
```

>没有优化用时：2.98s
>
>第一次优化用时：2.96s
>
>合并分支用时：3.22s

惊奇地发现，竟然有很大的性能下降！背后的原因之一写在了这一小节的标题中。这是一种刚刚我们的操作增加了控制冒险（**Control Hazard**）。合并分支后的代码每一次for循环都有一个分支，可能导致流水线冒险。这是现代处理器由于预测错误的分支而导致的性能下降。而第一段代码中for循环没有分支，分支预测可能会更准确。

简而言之，减少for循环中的分支对性能的提升帮助非常大！

值得一提的是，如果在Tiny-Renderer中使用本文的操作，速度将会进一步提升。这在Issues中也有相应讨论：[链接🔗](https://github.com/ssloy/tinyrenderer/issues/28)。

### 第七关：浮点数整型化

为什么我们必须用浮点数呢？在循环中我们只在与0.5做比较的时候用到了。因此我们完全可以将error乘个2再乘个dx（或dy），将其完全转化为int。

```c++
// 第二次优化的代码
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    int error2 = 0;
    if(std::abs(x0-x1)<std::abs(y0-y1)) { // “陡峭”线
        if (y0>y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        int dError2 = std::abs(dx) * 2;
        int x = x0;
        for (int y = y0; y <= y1; y++) {
            image.set(x, y, color);
            error2 += dError2;
            if (error2>dy) {
                x += (x1>x0?1:-1);
                error2 -= dy * 2;
            }
        }
    }else { // “平缓”线
        if (x0>x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        int dError2 = std::abs(dy) * 2;
        int y = y0;
        for (int x = x0; x <= x1; x++) {
            image.set(x, y, color);
            error2 += dError2;
            if (error2>dx) {
                y += (y1>y0?1:-1);
                error2 -= dx*2;
            }
        }
    }
}
```

>没有优化用时：2.98s
>
>第一次优化用时：2.96s
>
>合并分支用时：3.22s
>
>第二次优化用时：2.96s

优化程度也较为有限了，原因是在浮点数化整的过程中增加了计算的次数，与浮点数的计算压力相抵消了。

## 1.2 三维画线

在前面的内容中，我们完成了Line()函数的编写。具体内容是给定屏幕坐标上的两个点就可以在屏幕中绘制线段。

### 第一关：加载.obj

首先，我们创建model类作为物体对象。我们在model加载的.obj文件里可能会有如下内容：

```.obj
v 1.0 2.0 3.0
```

v表示3D坐标，后面通常是三个浮点数，分别对应空间中的x, y, z。上面例子代表一个顶点，其坐标为 `(1.0, 2.0, 3.0)`。

当定义一个面（`f`）时，你引用的是先前定义的顶点（`v`）的索引。

```.obj
f 1 2 3
f 1/4/1 2/5/2 3/6/3
```

上面两行都表示一个面，

- 第一行表示三个顶点的索引
- 第二行表示顶点/纹理坐标/法线的索引

在这里我提供一个简单的 .obj 文件解析器 model.cpp 。你可以在此处找到当前项目[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/1.2_Wireframe_rendering)。以下是你可能用到的model类的信息：

- 模型面数量：`i<model->nfaces()`
- 获取第n个面的三个顶点索引：`model->face(n)`
- 通过索引获取顶点三维坐标：`model->vert()`

> 本项目使用的.obj文件的所有顶点数据已做归一化，也就是说v后面的三个数字都是在[-1, 1]之间。

### 第二关：绘制

在这里我们仅仅考虑三维顶点中的(x, y)，不考虑深度值。最终在main.cpp中通过model解析出来的顶点坐标绘制出所有线框即可。

```c++
for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
    for (int j=0; j<3; j++) { 
        Vec3f v0 = model->vert(face[j]); 
        Vec3f v1 = model->vert(face[(j+1)%3]); 
        int x0 = (v0.x+1.)*width/2.; 
        int y0 = (v0.y+1.)*height/2.; 
        int x1 = (v1.x+1.)*width/2.; 
        int y1 = (v1.y+1.)*height/2.; 
        line(x0, y0, x1, y1, image, blue); 
    } 
}
```

这段代码对所有的面进行迭代，将每个面的三条边都进行绘制。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230831145239967.png" alt="image-20230831145239967" />

### 第三关：优化

将不必要的计算设置为const，避免重复分配释放内存。

```c++
const float halfWidth = screenWidth / 2.0f;
const float halfHeight = screenHeight / 2.0f;

int nfaces = model->nfaces();
for (int i = 0; i < nfaces; ++i) {
    const std::vector<int>& face = model->face(i);
    Vec3f verts[3];
    
    for (int j = 0; j < 3; ++j) {
        verts[j] = model->vert(face[j]);
    }

    for (int j = 0; j < 3; ++j) {
        const Vec3f& v0 = verts[j];
        const Vec3f& v1 = verts[(j + 1) % 3];
        
        int x0 = (v0.x + 1.0f) * halfWidth;
        int y0 = (v0.y + 1.0f) * halfHeight;
        int x1 = (v1.x + 1.0f) * halfWidth;
        int y1 = (v1.y + 1.0f) * halfHeight;
        
        line(x0, y0, x1, y1, image, blue);
    }
}

```

## 2.1 三角形光栅化

接下来，绘制完整的三角形，不光是一个个三角形线框，更是要一个实心的三角形！为什么是三角形而不是其他形状比如四边形？因为三角形可以任意组合成为所有其他的形状。基本上，在OpenGL中绝大多数都是三角形，因此我们的渲染器暂时无需考虑其他的东西了。

当绘制完一个实心的三角形后，完整渲染一个模型也就不算难事了。

在Games101的作业中，我们使用了AABB包围盒与判断点是否在三角形内的方法对三角形光栅化。你完全可以用自己的算法绘制三角形，在本文中，我们使用割半法处理。

### 第一关：线框三角形

利用上一章节完成的line()函数，进一步将其包装成绘制三角形线框的triangleLine()函数。

```c++
void triangleLine(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
    line(v0.u, v0.v, v1.u, v1.v, image, color);
    line(v0.u, v0.v, v2.u, v2.v, image, color);
    line(v1.u, v1.v, v2.u, v2.v, image, color);
}
...
triangleLine(Vec2i(0,0),Vec2i(25,25),Vec2i(50,0),image,red);
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230831145749059.png" alt="image-20230831145749059" style="zoom:200%;" />

### 第二关：请你自己画实心的三角形

这一部分最好由你自己花费大约一个小时完成。一个好的三角形光栅化算法应该是简洁且高效的。你目前的项目大概是这样的：[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/2.1_Filling_triangles)。

【此处省略一小时】

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230831213131128.png" alt="image-20230831213131128" style="zoom:200%;" />

### 第三关：扫描线算法

当你完成了你的算法之后，不妨来看看其他人是怎么做的。为了光栅化一个实心三角形，一种非常常见的方法是使用扫描线算法：

1. 按 `v`（或 `y`）坐标对三角形的三个顶点进行排序，使得 `v0` 是最低的，`v2` 是最高的。
2. 对于三角形的每一行（从 `v0.v` 到 `v2.v`），确定该行与三角形的两边的交点，并绘制一条从左交点到右交点的线。

```c++
void triangleRaster(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    if (v0.v > v1.v) std::swap(v0, v1);
    if (v0.v > v2.v) std::swap(v0, v2);
    if (v1.v > v2.v) std::swap(v1, v2);

    // Helper function to compute the intersection of the line and a scanline
    auto interpolate = [](int y, Vec2i v1, Vec2i v2) -> int {
        if (v1.v == v2.v) return v1.u;
        return v1.u + (v2.u - v1.u) * (y - v1.v) / (v2.v - v1.v);
    };

    for (int y = v0.v; y <= v2.v; y++) {
        // Intersect triangle sides with scanline
        int xa = interpolate(y, v0, v2); // Intersection with line v0-v2
        int xb = (y < v1.v) ? interpolate(y, v0, v1) : interpolate(y, v1, v2); // Depending on current half

        if (xa > xb) std::swap(xa, xb);

        // Draw horizontal line
        for (int x = xa; x <= xb; x++) {
            image.set(x, y, color);
        }
    }
}
```

### 第四关：包围盒逐点扫描

介绍另一个非常有名的方法，包围盒扫描方法。将需要光栅化的三角形框上一个矩形的包围盒子内，在这个包围盒子内逐个像素判断该像素是否在三角形内。如果在三角形内，则绘制对应的像素；如果在三角形外，则略过。伪代码如下：

```c++
triangle(vec2 points[3]) { 
    vec2 bbox[2] = find_bounding_box(points); 
    for (each pixel in the bounding box) { 
        if (inside(points, pixel)) { 
            put_pixel(pixel); 
        } 
    } 
}
```

想要实现这个方法，主要需要解决两个问题：找到包围盒、判断某个像素点是否在三角形内。

第一个问题很好解决，找到三角形的三个点中最小和最大的两个分量两两组合。

第二个问题似乎有些棘手。我们需要学习什么是重心坐标 （[barycentric coordinates](https://en.wikipedia.org/wiki/Barycentric_coordinate_system) ）。

### 第五关：重心坐标

利用重心坐标，可以判断给定某个点与三角形之间的位置关系。

给定一个三角形ABC和任意一个点P $(x,y)$ ，这个点的坐标都可以用点ABC线性表示。不理解也无所谓，简单理解就是一个点P和三角形三点的关系可以用三个数字来表示，像下面公式这样：
$$
P = (1-u-v)A+uB+vC
$$
我们把上面的式子解开，得到关于 $\overrightarrow{AB},\overrightarrow{AC}和\overrightarrow{AP}$的关系：
$$
P=A+u \overrightarrow{A B}+v \overrightarrow{A C}
$$
然后将点P挪到同一边，得到下面的式子：
$$
u \overrightarrow{A B}+v \overrightarrow{A C}+\overrightarrow{P A}=\overrightarrow{0}
$$
然后将上面的向量分为x分量与y分量，写成两个等式。接下来用矩阵表示他们：
$$
\left\{\begin{aligned}
{\left[\begin{array}{lll}
u & v & 1
\end{array}\right]\left[\begin{array}{l}
\overrightarrow{A B}_x \\
\overrightarrow{A C}_x \\
\overrightarrow{P A}_x
\end{array}\right]=0 } \\
{\left[\begin{array}{lll}
u & v & 1
\end{array}\right]\left[\begin{array}{l}
\overrightarrow{A B}_y \\
\overrightarrow{A C}_y \\
\overrightarrow{P A}_y
\end{array}\right]=0 }
\end{aligned}\right.
$$
两个向量点积是0，说明两个向量垂直。右边这俩向量都与 $[u v 1]$ ，说明他们的叉积就是$k[u v 1]$ ，因此轻轻松松解出uv。

梳理一下，当务之急是判断给定的一个点与一个三角形的关系。直接给出结论，如果点在三角形内部，则这三个系数都属于（0,1）之间。直接给出光栅化一个三角形的代码：

```c++
Vec3f barycentric(Vec2i v0, Vec2i v1, Vec2i v2, Vec2i pixel){
    // v0, v1, v2 correspond to ABC
    Vec3f u = Vec3f(v1.x-v0.x,// AB_x
                    v2.x-v0.x,// AC_x
                    v0.x-pixel.x)// PA_x
              ^
              Vec3f(v1.y-v0.y,
                    v2.y-v0.y,
                    v0.y-pixel.y);
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
// 重心坐标的方法 - 光栅化三角形
void triangleRaster(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
    // Find The Bounding Box
    Vec2i* pts[] = {&v0, &v1, &v2};// Pack
    Vec2i boundingBoxMin(image.get_width()-1,  image.get_height()-1);
    Vec2i boundingBoxMax(0, 0);
    Vec2i clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        boundingBoxMin.x = std::max(0, std::min(boundingBoxMin.x, pts[i]->x));
        boundingBoxMin.y = std::max(0, std::min(boundingBoxMin.y, pts[i]->y));

        boundingBoxMax.x = std::min(clamp.x, std::max(boundingBoxMax.x, pts[i]->x));
        boundingBoxMax.y = std::min(clamp.y, std::max(boundingBoxMax.y, pts[i]->y));
    }

    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec2i pixel;
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            image.set(pixel.x, pixel.y, color);
        }
    }
}
```

barycentric()函数可能比较难理解，可以暂时抛弃研究其数学原理。并且上面这段代码是经过优化的，如果希望了解其原理可以看我这一篇文章：[链接🔗](https://remoooo.com/cg/835.html)。

```c++
const int screenWidth  = 250;
const int screenHeight = 250;
...
triangleRaster(Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160),image,red);
```

![image-20230904005910991](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904005910991.png)

你可以在下面的链接中找到当前项目的代码：[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/2.1.1_barycentric_coordinates)。

## 2.2 平面着色Flat shading render

在「1.2 三维画线」中绘制了模型的线框，即空三角形模型。在「2.1 三角形光栅化」中，介绍了两种方法绘制一个“实心”的三角形。现在，我们将使用“平面着色”来渲染小人模型，其中平面着色使用随机的RGB数值。

### 第一关：回顾

首先将加载模型的相关代码准备好：

```c++
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

...
Model *model = NULL;
const int screenWidth  = 800;
const int screenHeight = 800;
...
    
// 光栅化三角形的代码
void triangleRaster(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color){
	...
}

int main(int argc, char** argv) {
    const float halfWidth = screenWidth / 2.0f;
    const float halfHeight = screenHeight / 2.0f;
    TGAImage image(screenWidth, screenHeight, TGAImage::RGB);
    model = new Model("../object/african_head.obj");

	...// 在此处编写接下来的代码

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
```

### 第二关：绘制随机的颜色

下面是遍历获得模型的每一个需要绘制的三角形的代码：

```c++
for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
	...
}
```

当我们获得了所有的面，在每一趟遍历中，将`face`的三个点取出来并转换到屏幕坐标上，最后传给三角形光栅化函数：

```c++
for (int j=0; j<3; j++) {
    Vec3f world_coords = model->vert(face[j]); 
    screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.); 
}
triangleRaster(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
```

![image-20230904134928856](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904134928856.png)

### 第三关：根据光线传播绘制颜色

刚才的随机颜色远远满足不了我们，现在我们根据光线与三角形的法线方向绘制不同的灰度。什么意思呢？看下面这张图，当物体表面的法线方向与光线方向垂直，物体接受到了最多的光；随着法线与光线方向的夹角越来越大，收到光的照射也会越来越少。当法线与光线方向垂直的时候，表面就接收不到光线了。

![image-20230904135449781](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904135449781.png)

将这个特性添加到光栅化渲染器中。

```c++
Vec3f light_dir(0,0,-1); // define light_dir

for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
    Vec2i screen_coords[3]; 
    Vec3f world_coords[3]; 
    for (int j=0; j<3; j++) { 
        Vec3f v = model->vert(face[j]); 
        screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
        world_coords[j]  = v; 
    } 
    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
    n.normalize(); 
    float intensity = n*light_dir; 
    if (intensity>0) { 
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
    } 
}
```

上面代码需要注意的点：

- 三角形法线`n`的计算
- 判断点积正负

`intensity`小于等于0的意思是这个面（三角形）背对着光线，摄像机肯定看不到，不需要绘制。

![image-20230904141708453](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904141708453.png)

注意到嘴巴的地方有些问题，本应在嘴唇后面的嘴巴内部区域（像口腔这样的空腔）却被画在嘴唇的上方或前面。这表明我们对不可见三角形的处理方式不够精确或不够规范。“dirty clipping”方法只适用于凸形状。对于凹形状或其他复杂的形状，该方法可能会导致错误。在下一章节中我们使用 z-buffer 解决这个瑕疵（渲染错误）。

这里给出当前步骤的代码[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/2.2_Flat_shading_render)。

## 3.1 表面剔除

上一章的末尾我们发现嘴巴部分的渲染出现了错误。本章先介绍画家算法（Painters' Algorithm），随后引出 Z-Buffer ，插值计算出需渲染的像素的深度值。

### 第一关：画家算法（Painters' Algorithm）

这个算法很直接，将物体按其到观察者的距离排序，然后从远到近的顺序绘制，这样近处的物体自然会覆盖掉远处的物体。

但是仔细想就会发现一个问题，当物体相互阻挡时算法就会出错。也就是说，画家算法无法处理相互重叠的多边形。

![image-20230904144410471](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904144410471.png)

### 第二关：了解z-buffer

如果画家算法行不通，应该怎么解决物体相互重叠的问题呢？我们初始化一张表，长宽与屏幕像素匹配，且每个像素大小初始化为无限远。每一个像素存储一个深度值。当要渲染一个三角形的一个像素时，先比较当前欲渲染的像素位置与表中对应的深度值，如果当前欲渲染的像素深度比较浅，说明欲渲染的像素更靠近屏幕，因此渲染。

而这张表，我们称之为：Z-Buffer。

### 第三关：创建Z-Buffer

理论上说创建的这个 Z-Buffer 是一个二维的数组，例如：

```c++
float **zbuffer = new float*[screenWidth];
for (int i = 0; i < screenWidth; i++) {
    zbuffer[i] = new float[screenHeight];
}
...
// 释放内存
for (int i = 0; i < screenWidth; i++) {
    delete[] zbuffer[i];
}
delete[] zbuffer;
```

但是，我认为这太丑陋了，不符合我的审美。我的做法是将二维数组打包变成一个一维的数组：

```c++
int *zBuffer = new int[screenWidth*screenHeight];
```

最基本的数据结构，取用的时候只需要：

```c++
int idx = x + y*screenWidth;
int x = idx % screenWidth;
int y = idx / screenWidth;
```

初始化zBuffer可以用一行代码解决，将其全部设置为负无穷：

```c++
for (int i=screenWidth*screenHeight; i--; zBuffer[i] = -std::numeric_limits<float>::max());
```

### 第四关：整理当前代码

要给当前的`triangleRaster()`函数新增 Z-Buffer 功能。

我们给`pixel`增加一个维度用于存储深度值。另外，由于深度是float类型，如果沿用之前的函数可能会出现问题，原因是之前传入的顶点都是经过取舍得到的整数且不包含深度信息。而且需要注意整数坐标下的深度值往往不等于取舍之前的深度值，这个精度的损失带来的问题是在复杂精细且深度值波动很大的位置会出现渲染错误。但是目前可以直接忽略，等到后面进行超采样、抗锯齿或者其他需要考虑像素内部细节的技术时再展开讲解。

因此，为了后期拓展的方便，我们将之前涉及pixel的Vec2i代码换为Vec3f类型，并且每一个点都增加一个维度用于存储深度值。

```c++
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2)
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1);
}
// 重心坐标的方法 - 光栅化三角形
void triangleRaster(Vec3f v0, Vec3f v1, Vec3f v2, float *zBuffer, TGAImage &image, TGAColor color){
    Vec3f* pts[] = {&v0, &v1, &v2};// Pack
    // Find The Bounding Box
    Vec2f boundingBoxMin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f boundingBoxMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        boundingBoxMin.x = std::max(0.f, std::min(boundingBoxMin.x, pts[i]->x));
        boundingBoxMin.y = std::max(0.f, std::min(boundingBoxMin.y, pts[i]->y));
        boundingBoxMax.x = std::min(clamp.x, std::max(boundingBoxMax.x, pts[i]->x));
        boundingBoxMax.y = std::min(clamp.y, std::max(boundingBoxMax.y, pts[i]->y));
    }

    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec3f pixel;// 将深度值打包到pixel的z分量上
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);// Screen Space
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            // HIGHLIGHT: Finished The Z-Buffer
            //image.set(pixel.x, pixel.y, color);
            pixel.z = 0;
            pixel.z = bc.x*v0.z+bc.y+v1.z+bc.z+v2.z;// 通过重心坐标插值计算当前Shading Point的深度值
            if(zBuffer[int(pixel.x+pixel.y*screenWidth)]<pixel.z) {
                zBuffer[int(pixel.x + pixel.y * screenWidth)] = pixel.z;
                image.set(pixel.x, pixel.y,color);
            }
        }
    }
}
```

将世界坐标转化到屏幕坐标的函数打包：

```c++
Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}
```

另外，对tgaimage、model和geometry做了一些修改，主要是优化了一些细节。具体项目请查看当前项目分支[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/3.1_Z-buffer)。

![image-20230904191612606](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230904191612606.png)

## 3.2 上贴图

啥是贴图呢？就是类似这种奇奇怪怪的图片。

![image-20230905174124334](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230905174124334.png)

目前我们已经完成了三角形的重心坐标插值得出了三角形内某点的深度值。接下来我们还可以用插值操作计算对应的纹理坐标。

本章基于「3.1 表面剔除」最后的项目完善，本章主要是c++ STL相关操作。

### 第一关：思路

请首先下载「3.1 表面剔除」最后的项目[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/3.1_Z-buffer)。

首先从硬盘中加载纹理贴图，然后传到三角形顶点处，通过对应的纹理坐标从texture获取颜色，最后插值得到各个像素的颜色。

另外，项目框架的代办清单：

1. 增加model模块中对vt标签的解析
2. 完善model模块中对f标签的解析，具体是获取纹理坐标索引
3. 完善geometry模块的操作符，具体是实现Vec<Dom, f>与float相乘等操作

### 第二关：加载纹理文件

从硬盘中加载纹理texture，用TGAImage存储。

```c++
TGAImage texture;
if(texture.read_tga_file("../object/african_head_diffuse.tga")){
    std::cout << "Image successfully loaded!" << std::endl;
    // 可以做一些图像处理
} else {
    std::cerr << "Error loading the image." << std::endl;
}
```

### 第三关：获取纹理坐标

在 model.h 中，在class Model上方创建一个Face结构体用于存储解析后obj中的f标签。f标签有三个值，这里只存储前两个。f标签的三个值分别是顶点索引/纹理索引/法线索引，等后面用到了法线坐标再拓展即可。

```c++
struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texcoordIndices;
    ...
};
```

然后将model的模版私有属性：

```c++
std::vector< std::vector<int> > faces_;
```

改为：

```c++
std::vector<Face> faces_;
```

同时也修改 model.cpp 下获取 face 的函数：

```c++
Face Model::face(int idx) {
    return faces_[idx];
}
```

实际解析时的函数：

```c++
else if (!line.compare(0, 2, "f ")) {
//            std::vector<int> f;
//            int itrash, idx;
//            iss >> trash;
//            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
//                idx--; // in wavefront obj all indices start at 1, not zero
//                f.push_back(idx);
//            }
//            faces_.push_back(f);
            Face face;
            int itrash, idx, texIdx;
            iss >> trash;
            while (iss >> idx >> trash >> texIdx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                texIdx--; // similarly for texture indices
                face.vertexIndices.push_back(idx);
                face.texcoordIndices.push_back(texIdx);
            }
            faces_.push_back(face);
        }
```

接下来解析纹理坐标索引texcoords_。

```c++
// model.h
...
class Model {
private:
	...
    std::vector<Vec2f> texcoords_;
public:
	...
    Vec2f& getTexCoord(int index);
};
...
```

```c++
// model.cpp
...
Model::Model(const char *filename) : verts_(), faces_(), texcoords_(){
    ...
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f tc;
            for (int i = 0; i < 2; i++) iss >> tc[i];
            texcoords_.push_back(tc);
        }
    ...
}
...    
Vec2f& Model::getTexCoord(int index) {
    return texcoords_[index];
}
```

最后就可以通过对应的索引得到纹理坐标了。

```c++
tex_coords[j] = model->getTexCoord(face.texcoordIndices[j]);
```

### 第四关：通过纹理坐标uv获取对应颜色

获得了纹理坐标后就可以用texture.get(x_pos, y_pos)获取图片（贴图/纹理）的对应像素。注意最后TGAColor使用的是BGRA通道，而不是RGBA通道。

```c++
TGAColor getTextureColor(TGAImage &texture, float u, float v) {
    // 纹理坐标限制在(0, 1)
    u = std::max(0.0f, std::min(1.0f, u));
    v = std::max(0.0f, std::min(1.0f, v));
    // 将u, v坐标乘以纹理的宽度和高度，以获取纹理中的像素位置
    int x = u * texture.get_width();
    int y = v * texture.get_height();
    // 从纹理中获取颜色
    TGAColor color = texture.get(x, y);
    // tga使用的是BGRA通道
    return TGAColor(color[2],color[1],color[0], 255);
}
```

### 第五关：在光栅化三角形函数中增加贴贴图的功能

增加了四个传参，分别是三个三角形的纹理坐标与纹理。实现细节直接看代码比较直接。

```c++
// 带贴图 - 光栅化三角形
void triangleRasterWithTexture(Vec3f v0, Vec3f v1, Vec3f v2,
                               Vec2f vt0, Vec2f vt1, Vec2f vt2,// 纹理贴图
                               float *zBuffer, TGAImage &image,
                               TGAImage &texture){
	...
    // Find The Bounding Box
	...
        
    // For Loop To Iterate Over All Pixels Within The Bounding Box
    Vec3f pixel;// 将深度值打包到pixel的z分量上
    for (pixel.x = boundingBoxMin.x; pixel.x <= boundingBoxMax.x; pixel.x++) {
        for (pixel.y = boundingBoxMin.y; pixel.y <= boundingBoxMax.y; pixel.y++) {
            Vec3f bc = barycentric(v0, v1, v2, pixel);// Screen Space
            if (bc.x<0 || bc.y<0 || bc.z<0 ) continue;
            // HIGHLIGHT: Finished The Z-Buffer
            pixel.z = 0;
            pixel.z = bc.x*v0.z+bc.y+v1.z+bc.z*v2.z;
            Vec2f uv = bc.x*vt0+bc.y*vt1+bc.z*vt2;
            if(zBuffer[int(pixel.x+pixel.y*screenWidth)]<pixel.z) {
                zBuffer[int(pixel.x + pixel.y * screenWidth)] = pixel.z;
                image.set(pixel.x, pixel.y,getTextureColor(texture, uv.x, 1-uv.y));
            }
        }
    }
}
```

在上面的代码中，你可能会发现乘号竟然报错了，这个问题在下一关马上得到解决。最终在 main() 函数中这样调用：

```c++
// main.cpp
...
for (int i=0; i<model->nfaces(); i++) {
    Face face = model->face(i);
    Vec3f screen_coords[3], world_coords[3];
    Vec2f tex_coords[3];
    for (int j=0; j<3; j++) {
        world_coords[j]  = model->vert(face.vertexIndices[j]);
        screen_coords[j] = world2screen(world_coords[j]);
        tex_coords[j] = model->getTexCoord(face.texcoordIndices[j]);
    }
    triangleRasterWithTexture(screen_coords[0], screen_coords[1], screen_coords[2],
                              tex_coords[0],tex_coords[1],tex_coords[2],
                              zBuffer, image, texture);
}
...
```

![image-20230905084358296](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230905084358296.png)

### 第六关：为模板函数添加更多重载符号操作

在写纹理坐标的时候，我们会用到一些操作比如说 Vec2i 类型与 float 浮点数相乘和相除。将下面的代码添加到 geometry.h 的中间部分：

```c++
...
    
template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

// -------------添加内容-------------
template<size_t DIM, typename T> vec<DIM, T> operator*(const T& scalar, const vec<DIM, T>& v) {
    vec<DIM, T> result;
    for (size_t i = 0; i < DIM; i++) {
        result[i] = scalar * v[i];
    }
    return result;
}

template<size_t DIM, typename T> vec<DIM, T> operator*(const vec<DIM, T>& v, const T& scalar) {
    vec<DIM, T> result;
    for (size_t i = 0; i < DIM; i++) {
        result[i] = v[i] * scalar;
    }
    return result;
}

template<size_t DIM, typename T> vec<DIM, T> operator/(const vec<DIM, T>& v, const T& scalar) {
    vec<DIM, T> result;
    for (size_t i = 0; i < DIM; i++) {
        result[i] = v[i] / scalar;
    }
    return result;
}

// -------------添加内容结束-------------

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM,T>& v) {
    for(unsigned int i=0; i<DIM; i++) {
        out << v[i] << " " ;
    }
    return out ;
}

...
```

这样就完全没问题了，大功告成。当然你也可以在这个[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/3.2_Diffuse_texture)中找到完整的代码。

## 4.1 透视视角

上文的内容全部都是正交视角下的渲染，这显然算不上酷，因为我们仅仅是将z轴“拍扁”了。这一章节的目标是学习绘制透视视角。

><img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/3aTJFQLzfWZuDOB.png" alt="image-20230409155021065" style="zoom: 33%;" />
>
>https://stackoverflow.com/questions/36573283/from-perspective-picture-to-orthographic-picture

### 第一关：线性变换

缩放可以表示为：
$$
\operatorname{scale}\left(s_x, s_y\right)=\left[\begin{array}{cc}
s_x & 0 \\
0 & s_y
\end{array}\right] .
$$
<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/pGTvkjmfCMlwRDs.png" alt="image-20230408154330557" style="zoom:50%;" />

拉伸可以表示为：
$$
\operatorname{shear-x}(s)=\left[\begin{array}{}
1 & s \\
0 & 1
\end{array}\right]
,
\operatorname{shear-y}(s)=\left[\begin{array}{}
1 & 0 \\
s & 1
\end{array}\right]
$$
<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/UIO1DCYtucQTKbg.png" alt="image-20230408154937046" style="zoom:50%;" />

旋转可以表示为：
$$
\mathbf{R}_\theta=\left[\begin{array}{cc}
\cos \theta & -\sin \theta \\
\sin \theta & \cos \theta
\end{array}\right]
$$
<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/MsPI1HRNmzxd6gS.png" alt="image-20230408155212728" style="zoom:50%;" />

### 第二关：齐次坐标 Homogeneous coordinates

为什么要引入齐次坐标呢？因为想要表示一个二维变换的平移并不能仅仅使用一个2x2的矩阵。平移并不在这个二维矩阵的线性空间中。因此，我们拓展一个维度帮助我们表示平移。

在计算机图形学中我们使用齐次坐标（Homogeneous Coord）。比如说一个二维的$(x, y)$使用平移矩阵变换到$(x', y')$：
$$
\left(\begin{array}{c}
x^{\prime} \\
y^{\prime} \\
w^{\prime}
\end{array}\right)=\left(\begin{array}{ccc}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{array}\right) \cdot\left(\begin{array}{l}
x \\
y \\
1
\end{array}\right)=\left(\begin{array}{c}
x+t_x \\
y+t_y \\
1
\end{array}\right)
$$
这样，我们就可以通过 $t_x, t_y$ 做平移变换，简直太聪明了。

在常规的笛卡尔坐标中，很难从数学表示上区分一个点和一个向量，因为它们都可能使用相同的形式如 vec2(x,y)。但在齐次坐标中，通过最后一个坐标值（这里的z）可以明确区分它们。当z=0时，它是一个向量；当z≠0时，它是一个点。较为数学一点的表示方法：
$$
k\left[\begin{array}{l}
x \\
y \\
1
\end{array}\right], k \neq 0
$$
上面公式中，无论 $k$ 取多少，都表示同一个点。再举个例子：
$$
\left[\begin{array}{c}
x \\
y \\
1
\end{array}\right] \equiv\left[\begin{array}{c}
2 x \\
2 y \\
2
\end{array}\right] \equiv\left[\begin{array}{c}
514 x \\
514 y \\
514
\end{array}\right] \equiv\left[\begin{array}{c}
114 x \\
114 y \\
114
\end{array}\right]
$$
齐次坐标是一个大大的好啊，当你进行数学操作时，结果的类型（向量或点）是明确的：

- **向量 + 向量 = 向量**：两个向量相加的结果仍然是一个向量。
- **向量 - 向量 = 向量**：两个向量相减的结果仍然是一个向量。
- **点 + 向量 = 点**：一个点和一个向量相加的结果是一个新的点。
- **点 + 点 = ？？**：两个点坐标的中点。

这使得数学操作更加直观和有意义。

> 一段来自屏幕外的声音🔊：齐次坐标最下面那行有啥用？？这个问题非常关键。

$$
\left[\begin{array}{lll}
a & b & m \\
c & d & n \\
p & q & 1
\end{array}\right]
$$

家喻户晓的，$\left[\begin{array}{ll}
a & b \\
c & d 
\end{array}\right]$可以实现缩放，$\left[\begin{array}{l}
m \\
n \\
1
\end{array}\right]$ 可以实现平移。

但是，这个 $\left[\begin{array}{ll}
p & q 
\end{array}\right]$ 能干嘛？

变换矩阵不做其他线性变换，仅仅将pq随便设为一个数：
$$
\left[\begin{array}{lll}
1 & 0 & 0 \\
0 & 1 & 0 \\
2 & 0 & 1
\end{array}\right]\left[\begin{array}{l}
x \\
y \\
1
\end{array}\right]=\left[\begin{array}{c}
x \\
y \\
2 x+1
\end{array}\right] \equiv\left[\begin{array}{c}
\frac{x}{2 x+1} \\
\frac{y}{2 x+1} \\
1
\end{array}\right]
$$
我们发现，这个变换有点奇怪。随着(x,y)越来越大，这个“缩放因子”就会越来越小。

> 有没有一种可能，这个就是近大远小？

![image-20230905160328502](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230905160328502.png)

没错，这就是一种透视的现象。至此，上面齐次坐标矩阵的最后一朵乌云已经攻破。 $\left[\begin{array}{ll}
p & q 
\end{array}\right]$ 就是用来做透视变换的。

随着最后一朵乌云散去，必然会迎来更多的乌云。新的乌云，名字叫做三维。

### 第三关：三维世界

上文所述都是二维下的，现在进入三维的世界。三维的齐次坐标自然就是用四维的矩阵表示。

缩放：
$$
\mathbf{S}\left(s_x, s_y, s_z\right)=\left(\begin{array}{cccc}
s_x & 0 & 0 & 0 \\
0 & s_y & 0 & 0 \\
0 & 0 & s_z & 0 \\
0 & 0 & 0 & 1
\end{array}\right)\\
$$
平移：
$$
\mathbf{T}\left(t_x, t_y, t_z\right)=\left(\begin{array}{cccc}
1 & 0 & 0 & t_x \\
0 & 1 & 0 & t_y \\
0 & 0 & 1 & t_z \\
0 & 0 & 0 & 1
\end{array}\right)
$$
绕x,z,y轴旋转：
$$
\left\{\begin{array}{rl}
\mathbf{R}_x(\alpha)=\left(\begin{array}{cccc}
1 & 0 & 0 & 0 \\
0 & \cos \alpha & -\sin \alpha & 0 \\
0 & \sin \alpha & \cos \alpha & 0 \\
0 & 0 & 0 & 1
\end{array}\right)\\
\mathbf{R}_z(\alpha)=\left(\begin{array}{cccc}
\cos \alpha & -\sin \alpha & 0 & 0 \\
\sin \alpha & \cos \alpha & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{array}\right)\\
\mathbf{R}_y(\alpha)=\left(\begin{array}{cccc}
\cos \alpha & 0 & \sin \alpha & 0 \\
0 & 1 & 0 & 0 \\
-\sin \alpha & 0 & \cos \alpha & 0 \\
0 & 0 & 0 & 1
\end{array}\right)
\end{array}\right.
$$
透视：
$$
\mathbf{P}\left(r\right)=\left(\begin{array}{cccc}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & r & 1
\end{array}\right)\\
$$
为什么只有z方向上才有r？因为我们默认摄像机摆在z轴，物体随着z轴透视缩放的。

现在，将一个三维坐标通过透视缩放，得到：
$$
\left[\begin{array}{llll}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & r & 1
\end{array}\right]\left[\begin{array}{l}
x \\
y \\
z \\
1
\end{array}\right]=\left[\begin{array}{c}
x \\
y \\
z \\
1+z r
\end{array}\right] \equiv\left[\begin{array}{c}
\frac{x}{1+z r} \\
\frac{y}{1+z r} \\
\frac{z}{1+z r} \\
1
\end{array}\right]
$$
![image-20230905173407444](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230905173407444.png)

上图中，横轴向左是z的正方向，纵轴向上是y的正方向。

根据相似三角形法则，y1/By=(c-z1)/c，最后得到：
$$
r=- \frac{1}{c}
$$
因此得到透视矩阵：
$$
\left[\begin{array}{cccc}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & -\frac{1}{c} & 1
\end{array}\right]
$$
大家可能发现，如果有接触过图形学的朋友们可能会对之前的学习产生怀疑。为什么这里顶点变换的透视变换矩阵和其他教材都不一样呢？比方说虎书上的透视矩阵是这样的：
$$
\left[\begin{array}{cccc}
\frac{1}{\text { aspect } \times \tan \left(\frac{f o v y}{2}\right)} & 0 & 0 & 0 \\
0 & \frac{1}{\tan \left(\frac{\text { fovy }}{2}\right)} & 0 & 0 \\
0 & 0 & -\frac{\text { far }+ \text { near }}{\text { far }- \text { near }} & -\frac{2 \times \text { far } \times \text { near }}{\text { far }- \text { near }} \\
0 & 0 & -1 & 0
\end{array}\right]
$$

- fovy：垂直视场角, 通常表示为度数。 

- aspect : 宽高比, 即视口宽度除以视口高度。 

- near : 近裁剪面的距离。 

- far: 远裁剪面的距离。

所以我们刚才推导出来的矩阵并不是常见的透视投影矩阵，但是他确实表达了投影的思想，因此我们暂时用着。

> 来自屏幕外的声音🔊：停停停，理论说了这么多，能不能搞点实践的！

### 第四关：具体代码实现

在上一关中，我们得到了不那么正规但是能用的透视矩阵，现在要做的就是将世界坐标的顶点转换到齐次坐标，然后乘上透视矩阵、视口矩阵。视口矩阵其实就是用一个简洁的矩阵把下面归一化设备坐标 NDC [-1,1]转换到了屏幕空间[0,width]。看下面这一段代码就是那个被ViewPort矩阵淘汰的家伙：

```c++
Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*screenWidth/2.+.5), int((v.y+1.)*screenHeight/2.+.5), v.z);
}
```

接下来，把顶点坐标乘上我们下面两个矩阵（顺序要注意）：

```c++
//初始化透视矩阵
Matrix Projection = Matrix::identity(4);
//初始化视角矩阵
Matrix ViewPort   = viewport(width/2, height/2, width/2, height/2);
//投影矩阵[3][2]=-1/c，c为相机z坐标
Projection[3][2] = -1.f/camera.z;
...
screen_coords[j] = m2v(ViewPort * Projection * v2m((world_coords[j])));
```

>一段来自屏幕外的声音🔊：等等等等，v2m和m2v是什么？viewport()具体实现方法是什么？

v2m是将向量变成矩阵（齐次坐标），m2v反之。

```c++
Vec3f m2v(Matrix m){
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}
```

然后还需要完善geometry的模块，在geometry.h中添加如下代码：

```c++
//////////////////////////////////////////////////////////////////////////////////////////////

const int DEFAULT_ALLOC=4;

class Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
    inline int nrows();
    inline int ncols();

    static Matrix identity(int dimensions);
    std::vector<float>& operator[](const int i);
    Matrix operator*(const Matrix& a);
    Matrix transpose();
    Matrix inverse();

    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
};

/////////////////////////////////////////////////////////////////////////////////////////////
...
// typedef mat<4,4,float> Matrix; 
```

然后添加文件 geometry.cpp 

```c++
//
// Created by remoooo on 2023/9/6.
//

#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include "geometry.h"

Matrix::Matrix(int r, int c) : m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), rows(r), cols(c) { }

int Matrix::nrows() {
    return rows;
}

int Matrix::ncols() {
    return cols;
}

Matrix Matrix::identity(int dimensions) {
    Matrix E(dimensions, dimensions);
    for (int i=0; i<dimensions; i++) {
        for (int j=0; j<dimensions; j++) {
            E[i][j] = (i==j ? 1.f : 0.f);
        }
    }
    return E;
}

std::vector<float>& Matrix::operator[](const int i) {
    assert(i>=0 && i<rows);
    return m[i];
}

Matrix Matrix::operator*(const Matrix& a) {
    assert(cols == a.rows);
    Matrix result(rows, a.cols);
    for (int i=0; i<rows; i++) {
        for (int j=0; j<a.cols; j++) {
            result.m[i][j] = 0.f;
            for (int k=0; k<cols; k++) {
                result.m[i][j] += m[i][k]*a.m[k][j];
            }
        }
    }
    return result;
}

Matrix Matrix::transpose() {
    Matrix result(cols, rows);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            result[j][i] = m[i][j];
    return result;
}

Matrix Matrix::inverse() {
    assert(rows==cols);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
    Matrix result(rows, cols*2);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            result[i][j] = m[i][j];
    for(int i=0; i<rows; i++)
        result[i][i+cols] = 1;
    // first pass
    for (int i=0; i<rows-1; i++) {
        // normalize the first row
        for(int j=result.cols-1; j>=0; j--)
            result[i][j] /= result[i][i];
        for (int k=i+1; k<rows; k++) {
            float coeff = result[k][i];
            for (int j=0; j<result.cols; j++) {
                result[k][j] -= result[i][j]*coeff;
            }
        }
    }
    // normalize the last row
    for(int j=result.cols-1; j>=rows-1; j--)
        result[rows-1][j] /= result[rows-1][rows-1];
    // second pass
    for (int i=rows-1; i>0; i--) {
        for (int k=i-1; k>=0; k--) {
            float coeff = result[k][i];
            for (int j=0; j<result.cols; j++) {
                result[k][j] -= result[i][j]*coeff;
            }
        }
    }
    // cut the identity matrix back
    Matrix truncate(rows, cols);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            truncate[i][j] = result[i][j+cols];
    return truncate;
}

std::ostream& operator<<(std::ostream& s, Matrix& m) {
    for (int i=0; i<m.nrows(); i++)  {
        for (int j=0; j<m.ncols(); j++) {
            s << m[i][j];
            if (j<m.ncols()-1) s << "\t";
        }
        s << "\n";
    }
    return s;
}
```

接下来，渲染器启动！

![image-20230906112817473](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230906112817473.png)

> 来自甲方的声音🔊：效果非常好，下次不要做了。

看得出来，画面出现了一点问题。但是值得注意的是，顶点的位置已经基本正确了。但是贴图出现了错误。

借此机会，调整一下贴图加载的逻辑。我们原先在main函数粗暴加载，现在我们将物体对应的贴图当作model对象的一个属性，自动读取。在model.h中加入字段：

```c++
TGAImage diffusemap_;
```

构造函数就可以根据文件名字存入对应的贴图了：

```c++
load_texture(filename, "_diffuse.tga", diffusemap_);
```

然后通过以下函数得到对应的uv坐标：

```c++
Vec2i Model::uv(int iface, int nvert) {
    int idx = faces_[iface][nvert][1];
    return Vec2i(uv_[idx].x*diffusemap_.get_width(), uv_[idx].y*diffusemap_.get_height());
}
```

改动部分比较多直接阅读项目吧，项目[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/4.1_MVP_matrix)在这里，我们在「4.2 代码分析」中详细讨论整个项目，力求搞懂每一行代码与设计思路，尤其是C++ STL细节。下面是一个最终结果：

![image-20230906183206037](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230906183206037.png)

## 4.2 项目代码分析

目前的代码[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/4.1_MVP_matrix)有较大的改动，但是技术原理是不变的。本章节可以选择性阅读，也可以直接跳到「5.1 移动摄像机」。

项目结构：

```tree
├── object
│   ├── african_head.obj
│   ├── african_head_diffuse.tga
├── CMakeLists.txt
├── geometry.cpp
├── geometry.h
├── main.cpp
├── model.cpp
├── model.h
├── tgaimage.cpp
└── tgaimage.h
```

### 第一关：model类

> model.h

```c++
class Model {
private:
    std::vector<Vec3f> verts_; // 模型的顶点
    std::vector<std::vector<Vec3i> > faces_; // this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_; // 存储模型的法线
    std::vector<Vec2f> uv_; // 存储模型的 UV 纹理坐标
    TGAImage diffusemap_; // 模型的漫反射纹理图像
    // load_texture() 在加载模型的时候会用到，用于加载纹理。
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename); // 构造函数，从给定文件名加载模型
    ~Model(); // 析构函数，用于释放模型所占用的资源
    int nverts(); // 返回模型的顶点数量
    int nfaces(); // 返回模型的面数量
    Vec3f vert(int i); // 返回指定索引的顶点
    Vec2i uv(int iface, int nvert); // 返回指定面和指定顶点的 UV 坐标
    TGAColor diffuse(Vec2i uv); // 根据给定的 UV 坐标，从纹理图中获取颜色
    std::vector<int> face(int idx); // 返回指定索引的面信息（可能是顶点/纹理坐标/法线的索引）
};
```

> Model.cpp

```c++
/*
 * 构造函数 Model::Model(const char *filename)
 * - 构造函数使用了初始化列表来对 verts_, faces_, norms_ 和 uv_ 进行初始化。
 *
 */
Model::Model(const char *filename) : verts_(), faces_(), norms_(), uv_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    /* 循环读取并解析每一行内容。
     * 根据行的开头字符来决定行的类型（例如顶点、法线、纹理坐标或面）。
     * 根据这些信息更新相应的成员变量。
     */
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i=0;i<3;i++) iss >> n[i];
            norms_.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i=0;i<2;i++) iss >> uv[i];
            uv_.push_back(uv);
        }  else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i=0; i<3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);// 调用 load_texture 函数加载相应的纹理文件
}

Model::~Model() {
}

// 返回模型的顶点数量
int Model::nverts() {
    return (int)verts_.size();
}
// 返回模型的面数量。
int Model::nfaces() {
    return (int)faces_.size();
}

// 接收一个面的索引并返回这个面的所有顶点/纹理/法线坐标
std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i=0; i<(int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
    return face;
}
// 返回指定索引的顶点。
Vec3f Model::vert(int i) {
    return verts_[i];
}
// 加载纹理。
void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}
// 返回给定 UV 坐标的漫反射颜色。
TGAColor Model::diffuse(Vec2i uv) {
    return diffusemap_.get(uv.x, uv.y);
}
// 返回指定面和顶点的 UV 坐标。
Vec2i Model::uv(int iface, int nvert) {
    int idx = faces_[iface][nvert][1];
    return Vec2i(uv_[idx].x*diffusemap_.get_width(), uv_[idx].y*diffusemap_.get_height());
}
```

### 第二关：geometry

> geometry.h中，分为两个部分：模版向量类，矩阵类

```c++
/* --向量类定义-- */
// t -> 任意类型的数据，比如说 int float double等等
template <class t> struct Vec2 {
    t x, y;// 创建了两个t类型的数据成员
    Vec2<t>() : x(t()), y(t()) {} // 使用类型t的默认构造函数来初始化x和y。
    Vec2<t>(t _x, t _y) : x(_x), y(_y) {} // 接受两个参数，初始化x,y
//    Vec2<t>(const Vec2<t> &v) : x(t()), y(t()) { *this = v; } // 模板类的拷贝构造函数
    Vec2<t>(const Vec2<t> &v) : x(v.x), y(v.y) {} // 我认为上面的代码不太好，改了一下
    Vec2<t> & operator =(const Vec2<t> &v) { // 重载了等号，改变符号左边对象的数值
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }
    Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
    Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }
    // 重载[]符号，这里官方写错了，if(x<=0)是错误的。
    t& operator[](const int i) { if (i<=0) return x; else return y; }
    // 重载输出流
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) { }
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class u> Vec3<t>(const Vec3<u> &v);
    Vec3<t>(const Vec3<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
    Vec3<t> & operator =(const Vec3<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }
        return *this;
    }
    Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
    t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    t& operator[](const int i) { if (i<=0) return x; else if (i==1) return y; else return z; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

// 为常用类型提供了类型别名
typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

/* 特化构造函数 */
// Vec3<int> <- Vec3<float>。浮点数值 -> 整数值
template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
// Vec3<float> <- Vec3<int>。整数值 -> 浮点数值
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);

// 使得Vec2对象可以被输出到输出流
template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}
// 使得Vec3对象可以被输出到输出流
template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Matrix类代表了一个浮点数矩阵，其中的数据存储在std::vector。
 *
 */

const int DEFAULT_ALLOC=4;

class Matrix {
    // 这是一个二维vector，它存储了矩阵的所有元素。
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    // 构造函数 可以接受行数和列数作为参数。如果没有手动提供参数，则会使用DEFAULT_ALLOC（默认值为4）来初始化。
    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
    // 这个inline语法比较适合用于短小且经常被调用的函数。比如 inline int add(int a, int b){return a+b;}
    inline int nrows();
    inline int ncols();

    static Matrix identity(int dimensions); // 返回给定维度的单位矩阵
    std::vector<float>& operator[](const int i); // 这是一个重载的[]操作符，使得你可以使用像mat[i]这样的语法来直接访问矩阵的行。
    Matrix operator*(const Matrix& a); // 重载*操作符，以支持矩阵乘法。
    Matrix transpose(); // 返回矩阵的转置
    Matrix inverse(); // 返回矩阵的逆

    friend std::ostream& operator<<(std::ostream& s, Matrix& m); // 支持流传输
};

/////////////////////////////////////////////////////////////////////////////////////////////

```

> geometry.cpp 大同小异，不做笔记了

注意到代码中用到了`remplate <> template <>`进行模版特化，这种用法我比较少见到，也看不懂。

假设现在有一个模版类`Vec2`，有一个函数模版`printType`：

```c++
template <typename T>
class TemplateClass {
public:
    template <typename U>
    void memberFunction() {
        std::cout << "General memberFunction\n";
    }
};
```

现在，要为`Vec2<float>`特化`printType`方法，让他在`int`的时候打印`int`的专属信息。

```c++
template <>
class Vec2<float> {
public:
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

    template <typename U>
    void printType();

    template <>
    void printType<int>() {
        std::cout << "Specialized Vec2<float> with int" << std::endl;
    }
};
```

测试：

```c++
int main() {
    Vec2<double> vecDouble;
    vecDouble.printType<int>();  // 输出: Vec2<double> with type int

    Vec2<float> vecFloat;
    vecFloat.printType<int>();   // 输出: Specialized Vec2<float> with int
    vecFloat.printType<double>();  // 输出: Vec2<float> with type double
}
```

### 第三关：main

```c++
#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;
const int depth  = 255;

Model *model = NULL;
int *zBuffer = NULL;
Vec3f light_dir(0,0,-1);
Vec3f camera(0,0,3);

// 4d-->3d
Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

// 3d-->4d
Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

// 视角矩阵
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Vec3f barycentric(Vec3i *pts, Vec3i P) {
    Vec3f u =
            Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^
            Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y)
    ;
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
void triangle(Vec3i t0, Vec3i t1, Vec3i t2,
              Vec2i uv0, Vec2i uv1, Vec2i uv2,
              TGAImage &image, float intensity, int *zbuffer) {
    // Compute bounding box of the triangle
    Vec3i bboxmin(image.get_width() - 1, image.get_height() - 1, 0);
    Vec3i bboxmax(0, 0, 0);
    Vec3i clamp(image.get_width() - 1, image.get_height() - 1, 0);
    bboxmin.x = std::max(0, std::min(bboxmin.x, std::min(t0.x, std::min(t1.x, t2.x))));
    bboxmin.y = std::max(0, std::min(bboxmin.y, std::min(t0.y, std::min(t1.y, t2.y))));
    bboxmax.x = std::min(clamp.x, std::max(t0.x, std::max(t1.x, t2.x)));
    bboxmax.y = std::min(clamp.y, std::max(t0.y, std::max(t1.y, t2.y)));
    Vec3i pts[3] = {t0, t1, t2};
    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;  // P is outside of triangle
            P.z = bc_screen.x * t0.z + bc_screen.y * t1.z + bc_screen.z * t2.z;
            Vec2i uvP;
            uvP.x = bc_screen.x * uv0.x + bc_screen.y * uv1.x + bc_screen.z * uv2.x;
            uvP.y = bc_screen.x * uv0.y + bc_screen.y * uv1.y + bc_screen.z * uv2.y;
            int idx = P.x + P.y * image.get_width();
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y,
                          TGAColor(color.r * intensity, color.g * intensity, color.b * intensity));
            }
        }
    }
}

int main(int argc, char** argv) {
    model = new Model("../object/african_head.obj");
    zBuffer = new int[width*height];

    for (int i=width*height; i--; zBuffer[i] = -std::numeric_limits<float>::max());

    { // draw the model
        Matrix Projection = Matrix::identity(4);
        Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
        Projection[3][2] = -1.f/camera.z;

        TGAImage image(width, height, TGAImage::RGB);
        for (int i=0; i<model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            for (int j=0; j<3; j++) {
                Vec3f v = model->vert(face[j]);
                // 视角矩阵*投影矩阵*坐标
                screen_coords[j] =  m2v(ViewPort*Projection*v2m(v));
                world_coords[j]  = v;
            }
            // 计算法向量并且标准化
            Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]).normalize();
            // 计算光照
            float intensity = n*light_dir;
            if (intensity>0) {
                Vec2i uv[3];
                for (int k=0; k<3; k++) {
                    uv[k] = model->uv(i, k);
                }
                // 绘制三角形
                triangle(screen_coords[0], screen_coords[1], screen_coords[2],
                         uv[0], uv[1], uv[2], image, intensity, zBuffer);
            }
        }

        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    { // 输出zbuffer
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i=0; i<width; i++) {
            for (int j=0; j<height; j++) {
                zbimage.set(i, j, TGAColor(zBuffer[i+j*width], 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete [] zBuffer;
    return 0;
}
```

## 5.1 移动摄像机

能看到这个地方的朋友简直太牛逼了，这一章就稍微有点简单了。

在初中我们就学过，物体的运动是相对的。

摄像机向左移动，其实就是物体向右移动。我们可以让摄像机保持不动，让物体运动（平移）。

旋转其实也是一样的，假设摄像机绕着垂直的y轴顺时针旋转了45度，相当于物体逆时针旋转了45度。

图形学中我们知道，一个变换就代表了一个矩阵。并且在线性代数中我们知道，一个矩阵A乘上另一个矩阵B之后，想要变回原来的矩阵A，只需要再乘上矩阵B的逆。也就是说，要反变换，只需要乘上对应变换的逆即可。

### 第一关：定义摄像机

我们想，需要定义什么量才可以确保摄像机在同一时间拍摄的画面是一模一样的？（这里只考虑摄像机的位置、方向，摄像机镜头、焦距、ISO等信息默认相等。）

很显然可以想到的是摄像机的位置，我们将其定义为`cameraPos`。

并且拍摄的方向也需要定义，我们就叫他`gazeDirection`。

大家应该都玩过Rainbow Six围攻或者是PUBG吧，他们都有一个共同的特点：可以歪头射击。即使是在同一个摄像机位置，同一个注视方向，也不能确保拍摄画面的唯一性。因此，我们定义一个「向上向量」，即`viewUp`。

拓展一下，「向上向量」的很多好处：

1. 当使用欧拉角定义摄像机旋转时，万向节锁是一个常见问题，它会导致摄像机失去一个自由度的旋转。使用向上向量和观察向量可以防止这个问题，因为它们定义了一个明确的摄像机方向和旋转。
2. 有了向上向量，我们可以方便地使用叉积计算摄像机的右向量，这对于某些计算和操作非常有用。

ok，完事之后，我们开始写代码。

### 第二关：相机代码

在上一关我们得到了一个相机的基本定义：位置、看向的方向以及向上向量。

但是「看向的方向」这个量不太直接，我们想要更直接的也就是摄像机想要看向哪里，于是相机的代码中，定义相机的三要素就是：位置（`cameraPos`），看向的位置（`lookAt`）以及向上向量（`viewUp`）。

阅读下面代码之前需要知道我们注视的方向是 `-z` 轴。

```c++
Matrix lookAt(Vec3f cameraPos, Vec3f lookAt, Vec3f viewUp){
    //计算出z，根据z和up算出x，再算出y
    Vec3f gazeDirection = (cameraPos - lookAt).normalize(); // -z 轴
    Vec3f horizon = (viewUp ^ gazeDirection).normalize(); // 与相机水平的轴
    Vec3f vertical = (gazeDirection ^ horizon).normalize(); //

    Matrix rotation = Matrix::identity(4);
    Matrix translation = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        rotation[i][3] = - lookAt[i];
        rotation[0][i] = horizon[i];
        rotation[1][i] = vertical[i];
        rotation[2][i] = gazeDirection[i];
    }
    //这样乘法的效果是先平移物体，再旋转
    Matrix res = rotation*translation;
    return res;
}
```

![image-20230907165837308](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230907165837308.png)

## 6.1 优化/重写代码

首先继续优化 geometry 类，相当于重写一个自己的向量类。接着将当前main函数的一系列关于光栅化三角形的代码整合到新的类中。

由于本章的每一关内容都很多，尤其是重写模版向量类，因此我将内容较多的关卡提高了标题层次。



## 6.2 重写模版向量类

### 第一关：需求分析

不懂c++模版类的读者可以阅读 **附件1** ，不看也行，但是确保你对C++的模版特性有所了解。

本文我一步一步带大家实现以下功能：

1. **向量 (vec)**
   - 有通用的模板定义和2D、3D的特化版本。
   - 通用构造函数，将每个元素初始化为T类型的默认值。
   - 2D和3D向量的构造函数可以接受特定的初始化值。
   - 提供了索引运算符来获取或设置特定元素的值。
   - 3D向量有`norm`函数，返回向量的模长。
   - 3D向量有`normalize`函数，可以规范化向量。
   - 重载了输出运算符，方便向量的打印。
   - 运算符重载：向量的点乘、加法、减法、标量乘法和标量除法。
   - `embed`和`proj`函数用于扩展或投影向量到不同维度。
   - 3D向量之间的外积运算。
2. **矩阵 (mat)**
   - 可以获取或设置矩阵的行。
   - 获取矩阵的某一列。
   - 设置矩阵的某一列。
   - 获取单位矩阵。
   - 计算矩阵的行列式。
   - 获取矩阵的子矩阵。
   - 计算矩阵的余子式。
   - 计算伴随矩阵。
   - 计算逆矩阵的转置。
   - 运算符重载：矩阵和向量的乘法、两个矩阵的乘法、矩阵的标量除法。
   - 重载了输出运算符，方便矩阵的打印。
3. **其他功能**
   - 使用typedef定义了常用的类型，如`Vec2f`, `Vec3i`, `Matrix`等。
   - 在geometry.cpp中，提供了从3D和2D的float向量到int向量的转换，以及相反的转换。



### 第二关：实现Vec2模版以及四个算数符

这一关我们构建Vec2i和Vec2f类，以及实现他们的加、减、点积和叉积操作。

我这里直接创建了一个新的cpp项目，名为MyMathLib。在项目中，创建 `geometry.h` 头文件和 `geometry.cpp` 源代码文件。**提醒一下**，其实在写类模版的时候尽量都把内容写在头文件里即可，这里只是暂时分开写，我们马上就会发现这种写法的维护难度很大。

在头文件中定义 Vec2 模版类，让他既支持整形，也支持浮点数。

```c++
// .h file
#ifndef MYMATHLIB_GEOMETRY_H
#define MYMATHLIB_GEOMETRY_H

// geometry.h
#pragma once

#include <iostream>

template <typename T>
struct Vec2 {
    T x, y;

    Vec2();
    Vec2(T x, T y);

    Vec2<T> operator+(const Vec2<T>& v) const;
    Vec2<T> operator-(const Vec2<T>& v) const;
    T dot(const Vec2<T>& v) const;
    T cross(const Vec2<T>& v) const;
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;



#endif //MYMATHLIB_GEOMETRY_H
```

源代码文件实现构造函数、加法、减法、点积和叉积，然后在最后实现外部模板实例化。

**注意**：一般情况下，我们都直接把所有的实现（即函数体）都放在头文件中，这里只是稍微拓展一下可以使用**外部模板实例化**将类的模版的实现放在.cpp中。

**首先，我们需要理解C++中的模板是什么。**模板不是实际的函数或类，而是编译器使用的蓝图，用于生成函数或类的特定版本。这就是为什么我们通常会看到模板的定义直接在头文件中：当模板在某个源文件中使用时，编译器需要看到完整的模板定义，以便为特定的类型生成正确的代码。

**为什么要实现外部模板实例化？**模板的定义通常直接出现在头文件中。但有时，为了组织或其他原因，会把模板类的定义从其声明中分离出来（就像常规的非模板类那样），我目前也是这样做的。但这样做引发了一个问题，当链接器尝试链接对象文件时，如果它没有为特定的模板类型实例找到定义，就会出错。这是因为编译器只为那些它确实看到的模板类型生成代码。

对于模板类，如果模板类的所有成员函数都在类声明中定义（即在头文件中定义），那么当模板类用于特定类型时，编译器可以立即为该类型生成模板类的实例。但是，如果模板类的某些成员函数在类声明之外定义（例如，在`.cpp`文件中），那么你可能需要使用外部模板实例化来确保为所需的类型生成正确的模板实例。

```c++
// geometry.cpp
#include "geometry.h"

// 构造函数
template <typename T>
Vec2<T>::Vec2() : x(0), y(0) {}

template <typename T>
Vec2<T>::Vec2(T x, T y) : x(x), y(y) {}

// 加法
template <typename T>
Vec2<T> Vec2<T>::operator+(const Vec2<T>& v) const {
    return Vec2<T>(x + v.x, y + v.y);
}

// 减法
template <typename T>
Vec2<T> Vec2<T>::operator-(const Vec2<T>& v) const {
    return Vec2<T>(x - v.x, y - v.y);
}

// 点积
template <typename T>
T Vec2<T>::dot(const Vec2<T>& v) const {
    return x * v.x + y * v.y;
}

// 叉积
template <typename T>
T Vec2<T>::cross(const Vec2<T>& v) const {
    return x * v.y - y * v.x;
}

template class Vec2<int>;
template class Vec2<float>;
template class Vec2<double>;
```

调用测试一下。

```c++
// main.cpp
#include "geometry.h"
#include <iostream>

int main() {
    // 使用浮点数向量
    Vec2f v1(1.0f, 2.0f);
    Vec2f v2(2.0f, 3.0f);

    Vec2f sum = v1 + v2;
    std::cout << "v1 + v2 = (" << sum.x << ", " << sum.y << ")\n";

    float dotProduct = v1.dot(v2);
    std::cout << "v1 . v2 = " << dotProduct << "\n";

    float crossProduct = v1.cross(v2);
    std::cout << "v1 x v2 = " << crossProduct << "\n";

    // 使用整数向量
    Vec2i v3(1, 2);
    Vec2i v4(2, 3);

    Vec2i sumInt = v3 + v4;
    std::cout << "v3 + v4 = (" << sumInt.x << ", " << sumInt.y << ")\n";

    int dotProductInt = v3.dot(v4);
    std::cout << "v3 . v4 = " << dotProductInt << "\n";

    int crossProductInt = v3.cross(v4);
    std::cout << "v3 x v4 = " << crossProductInt << "\n";

    return 0;
}

```

结果：

>v1 + v2 = (3, 5)
>v1 . v2 = 8
>v1 x v2 = -1
>v3 + v4 = (3, 5)
>v3 . v4 = 8
>v3 x v4 = -1

### 第三关：实现Vec3模版以及四个算数符

Vec3其实和Vec2基本一致，只需修改一下计算代码即可，这里就不一一展示了。大部分就是将Vec2改成Vec3，计算时增加一个维度的考虑，比方说叉积。

```c++
template <typename T>
Vec3<T> Vec3<T>::cross(const Vec3<T> &v) const {
    return Vec3<T>(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
    );
}
```

### 第四关：用模版构建不同大小的向量

如果我还想添加Vec4，岂不是又要写一大堆，不简洁！对于多种不同大小的向量进行明确实例化是非常繁琐的。

这里我是用的是 `...` 折叠表达式（C++17）递归构造。目前头文件大致结构是这样的：

```c++
template <typename T, int N>
struct Vec {
    T values[N];

    // 构造函数
    Vec() = default; // 默认构造函数
    template<typename... Args> Vec(Args... args);
    
    // ... 操作声明（例如加减点积等） ...
};

// ... 实现 ...

// 为 Vec2、Vec3、Vec4 等提供类型别名

```

这里说一下构造函数，如果是隐式构造的，那么默认会使用Vec()。为了代码健壮性，其实可以在带可变长参数的构造函数前加 `explicit` 关键词。

```c++
template<typename... Args> explicit Vec(Args... args);
```

读者可能感觉到了，我在这里直接将 .cpp 的操作挪过来头文件里边来实现了，这是因为如果这个时候要分离写的话，代码冗余量会很大。因此我们全部写在头文件里面，省事优雅。下面是当前完整的 geometry.h 代码。

```c++
#ifndef MYMATHLIB_GEOMETRY_H
#define MYMATHLIB_GEOMETRY_H

// geometry.h
#pragma once

#include <iostream>

template <typename T, int N>
struct Vec {
    T values[N];

    // 构造函数
    Vec() = default; // 默认构造函数
    template<typename... Args> explicit Vec(Args... args);

    void print() const;
    T& operator[](int index);
    const T& operator[](int index) const;

    Vec<T, N> operator+(const Vec<T, N>& other) const;
    Vec<T, N> operator-(const Vec<T, N>& other) const;
    T dot(const Vec<T, N>& other) const;

    // 叉积仅对3D向量有效
    template<int M = N>
    typename std::enable_if<M == 3, Vec<T, 3>>::type cross(const Vec<T, 3>& other) const;
};

template <typename T, int N>
template<typename... Args>
Vec<T, N>::Vec(Args... args) : values{args...} {
    static_assert(sizeof...(args) == N, "Wrong number of arguments");
}

// 打印Vec
template <typename T, int N>
void Vec<T, N>::print() const {
    std::cout << "(";
    for(int i = 0; i < N; i++) {
        std::cout << values[i] << (i < N - 1 ? ", " : ")\n");
    }
}

// 非常量的Vec访问，可以称为左值
template <typename T, int N>
T& Vec<T, N>::operator[](int index) {
    return values[index];
}

// 常量Vec访问
template <typename T, int N>
const T& Vec<T, N>::operator[](int index) const {
    return values[index];
}

// 实现加法运算
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator+(const Vec<T, N>& other) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] + other[i];
    }
    return result;
}

// 实现减法运算
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator-(const Vec<T, N>& other) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] - other[i];
    }
    return result;
}

// 实现点积运算
template <typename T, int N>
T Vec<T, N>::dot(const Vec<T, N>& other) const {
    T sum = 0;
    for(int i = 0; i < N; i++) {
        sum += values[i] * other[i];
    }
    return sum;
}

// 实现了Vec3的叉积运算
template <typename T, int N>
template<int M>
typename std::enable_if<M == 3, Vec<T, 3>>::type Vec<T, N>::cross(const Vec<T, 3>& other) const {
    return Vec<T, 3>(
            values[1] * other[2] - values[2] * other[1],
            values[2] * other[0] - values[0] * other[2],
            values[0] * other[1] - values[1] * other[0]
    );
}

// 为 Vec2、Vec3、Vec4 等提供类型别名
using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;

using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;

using Vec2d = Vec<double, 2>;
using Vec3d = Vec<double, 3>;
using Vec4d = Vec<double, 4>;


#endif //MYMATHLIB_GEOMETRY_H

```

测试：

```c++
int main() {
    // 使用整数向量测试
    Vec3i v3(1, 2, 3);
    Vec3i v4(2, 3, 4);

    Vec3i sumInt = v3 + v4;
    sumInt.print();

    int dotProductInt = v3.dot(v4);
    std::cout << "v3 \\dot v4 = " << dotProductInt << "\n";

    Vec3i crossProduct = v3.cross(v4);
    std::cout << "v3 x v4 = "; crossProduct.print();
    
    return 0;
}
```

>(3, 5, 7)
>v3 \dot v4 = 20
>v3 x v4 = (-1, 2, -1)



### 第五关：进一步完善向量功能

先总结一下目前完成的内容：

- 通过两个模版参数 T 和 N，定义了一个向量。
- 有默认构造函数与可变参数的构造函数。
- 功能有：打印向量、通过索引访问向量元素、向量加法、向量减法、向量点积和三维向量叉积。
- 提供了大量常用的向量别名。

目前来说已经基本可以用了，但是还有很多需要完善，我们继续看需要完成的内容！

1. 增加标量与向量的乘/除法
2. 计算向量的模
3. 向量单位化
4. 重载输出运算符

另外，可以在声明运算操作中使用 `[[nodiscard]]` 标签，提醒编译器注意检查返回值是否得到使用，然后使用该库的用户就可以在编辑器中得到提醒，例如下面。

```c++
[[nodiscard]] Vec<T, N> normalize() const;// 将向量单位化
```

当前功能的声明：

```c++
[[nodiscard]] Vec<T, N> operator*(T scalar) const;// 向量与常数乘法
[[nodiscard]] Vec<T, N> operator/(T scalar) const;// 向量与常数除法
[[nodiscard]] double magnitude() const;// 向量模长
[[nodiscard]] Vec<T, N> normalize() const;// 向量单位化
// 流传输功能
template <typename U, int M>
friend std::ostream& operator<<(std::ostream& os, const Vec<U, N>& vec);
```

对应的实现：

```c++
...
// 流传输功能
template <typename U, int M>
std::ostream& operator<<(std::ostream& os, const Vec<U, M>& vec) {
    os << "(";
    for(int i = 0; i < M; i++) {
        os << vec[i];
        if (i < M - 1) {
            os << ", ";
        }
    }
    os << ")";
    return os;
}

// 实现标量与向量的乘
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator*(T scalar) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] * scalar;
    }
    return result;
}

// 实现标量与向量的除法
template <typename T, int N>
Vec<T, N> Vec<T, N>::operator/(T scalar) const {
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] / scalar;
    }
    return result;
}

// 实现求模长
template <typename T, int N>
double Vec<T, N>::magnitude() const {
    T sum = 0;
    for(int i = 0; i < N; i++) {
        sum += values[i] * values[i];
    }
    return std::sqrt(sum);
}

// 单位化向量
template <typename T, int N>
Vec<T, N> Vec<T, N>::normalize() const {
    T mag = magnitude();
    if(mag == 0) {
        // 不能单位化一个零向量，此处可以抛出异常或返回原向量
        // 为简化，此处返回原向量
        return *this;
    }
    Vec<T, N> result;
    for(int i = 0; i < N; i++) {
        result[i] = values[i] / mag;
    }
    return result;
}
```

可以在这里[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/Custom_Math_Lib)中获取当前的向量库代码。

### 第六关：构建矩阵

有了上面构建向量模版的经验，我们可以照葫芦画瓢写出一个矩阵模版。矩阵的构造、访问元素、加法、乘法等操作都一一实现即可。

这里读者应该给自己几个小时，独立写出代码。

当我作为库的使用者创建一个矩阵时，我会想这样创建：

```c++
Matrix<int, 2, 2> mat = {
    {1, 2},
    {3, 4}
};
```

构造函数可以使用两层嵌套的 `std::initializer_list`。其中，`std::initializer_list` 是一个C++11中引入的模板类，它表示编译时确定的值列表。先遍历行，再遍历列。

```c++
Matrix(const std::initializer_list<std::initializer_list<T>>& list) {
    int r = 0;
    for (const auto& rowList : list) {
        int c = 0;
        for (const auto& val : rowList) {
            values[r][c] = val;
            c++;
        }
        r++;
    }
}
// BTW：奇技淫巧压缩代码
Matrix(const std::initializer_list< std::initializer_list<T> >& list) {
    T* target = &values[0][0];
    for (const auto& rowList : list) {
        target = std::copy(rowList.begin(), rowList.end(), target);
    }
}
```

这里重点说一下矩阵的乘法。

```c++
template <typename T, int Rows, int Cols>
struct Matrix {
    T values[Rows][Cols];
    
	...
        
	// 矩阵与矩阵的乘法
    template<int NewCols>
    Matrix<T, Rows, NewCols> operator*(const Matrix<T, Cols, NewCols>& other) const {
        Matrix<T, Rows, NewCols> result;
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < NewCols; j++) {
                T sum = 0;
                for (int k = 0; k < Cols; k++) {
                    sum += values[i][k] * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }
};
```

一个 `Rows x Cols` 的矩阵A和一个 `Cols x NewCols` 的矩阵B相乘，那么结果将是一个 `Rows x NewCols` 的矩阵。举个例子：

```c++
Matrix<int, 3, 2> matA = { /* 初始化 */ };
Matrix<int, 2, 4> matB = { /* 初始化 */ };
Matrix<int, 3, 4> result = matA * matB;  // 这里的乘法使用的就是上述函数，NewCols 在这里为4
```

下面是其他的一些操作。

```c++
// 打印矩阵函数
void print() const {
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            std::cout << values[i][j];
            if (j < Cols - 1) {
                std::cout << "\t";  // 在列之间添加制表符，以美化输出
            }
        }
        std::cout << std::endl;  // 打印换行，进入下一行
    }
}

// 访问矩阵元素
T& operator()(int row, int col) {
    return values[row][col];
}
const T& operator()(int row, int col) const {
    return values[row][col];
}

// 矩阵加法
Matrix operator+(const Matrix& other) const {
    Matrix result;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            result(i, j) = values[i][j] + other(i, j);
        }
    }
    return result;
}

// 矩阵与标量的乘法
Matrix operator*(T scalar) const {
    Matrix result;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            result(i, j) = values[i][j] * scalar;
        }
    }
    return result;
}
```

总结一下目前的工作：

- 构建了矩阵类的模版结构
- 提供默认和可传参的构造函数
- 打印矩阵函数
- 访问矩阵操作符()
- 矩阵加法+
- 矩阵与标量的乘法*
- 矩阵与矩阵的乘法*

添加了完整的注释供大家参考，可以在这个[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/Custom_Math_Lib_v2)中找到当前的数学库。

### 第七关：继续完善矩阵库

- 添加单位矩阵identity

```c++
// 添加单位矩阵功能
static Matrix<T, Rows, Cols> identity();
...
// 实现
template <typename T, int Rows, int Cols>
Matrix<T, Rows, Cols> Matrix<T, Rows, Cols>::identity() {
    static_assert(Rows == Cols, "Identity matrix can only be created for square matrices.");
    Matrix<T, Rows, Cols> mat = {}; // Initialize all elements to zero
    for (int i = 0; i < Rows; ++i) {
        mat(i, i) = 1;
    }
    return mat;
}
```

- 插入列

```c++
void set_col(size_t idx, const Vec<T, Rows>& v) const;
...
template<typename T, int Rows, int Cols>
void Matrix<T, Rows, Cols>::set_col(size_t idx, const Vec<T, Rows>& v) const{
    assert(idx < Cols);// 编译器先判断需要设置的行是否合理
    for (int i = 0; i < Rows; i++) {
        values[i][idx] = v[i];
    }
}
```

- 插入列

```c++
void set_row(size_t idx, const Vec<T, Rows>& v);
...
// 添加插入行向量到矩阵的功能
template<typename T, int Rows, int Cols>
void Matrix<T, Rows, Cols>::set_row(size_t idx, const Vec<T, Rows>& v){
    assert(idx < Cols);// 编译器先判断需要设置的行是否合理
    for (size_t j = 0; j < Rows; j++) {
        values[idx][j] = v[j];
    }
}
```

测试：

```c++
Matrix4f m = Matrix4f::identity();
const Vec4f vec4F(2,2,2,2);
m.set_col(1,vec4F);
m.set_row(3, vec4F);
m.print();
return 0;
```

输出：

>1	2	0	0
>0	2	0	0
>0	2	1	0
>2	2	2	2

- 重载 `[][]` 

现在我如果要取用 Matrix 的 mat 对象的数值，我们是这样的

```c++
mat.values[][]
```

但是我想直接

```c++
mat[][]
```

此时我们需要使用代理对象的设计模式：

```c++
template <typename T, int Rows, int Cols>
struct Matrix {
    T values[Rows][Cols];

    // ... 其他成员函数和数据 ...

    // 代理对象
    struct RowProxy {
        T* row;
        T& operator[](int col) {
            return row[col];
        }
    };

    RowProxy operator[](int row) {
        return RowProxy{values[row]};
    }

    RowProxy operator[](int row) const {
        return RowProxy{values[row]};
    }
};
```







## 6.3 整合光栅化代码

浏览我们目前的main函数，既有矩阵变换函数，也有视角变换函数，还有三角形重心坐标光栅化三角形的函数，更有视角变换矩阵等，真的有些乱。我们将这些方法打包到一个新的类里面，这个类称为：our_gl。

### 特别节目1之：main代码之旅

最终main函数如下：

```c++
#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f       eye(0,-1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
};

int main(int argc, char** argv) {

    model = new Model("../object/african_head/african_head.obj");

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    GouraudShader shader;
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
```

![image-20230921171158181](https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921171158181.png)

其中，顶点着色和片元着色是可编程的。可以参考当前的项目代码[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/6_1_Our_GL)。

🎬 **开场白**

接下来开始解读这个main函数。首先，代码导入了一堆头文件，为了让我们的程序能够处理3D模型、向量计算和图像生成。

```c++
#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
```

🌍 **全局变量来啦**

接着，全局变量闪亮登场！有了宽度、高度、光照方向、观察点等等，这简直是个小型的“宇宙”。

```c++
Model *model     = NULL;
const int width  = 800;
const int height = 800;
Vec3f light_dir(1,1,1);
Vec3f       eye(0,-1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
```

🎭 **GouraudShader 诞生**

然后，我们有一个名为 `GouraudShader` 的类，这家伙是渲染的明星！它的职责是处理顶点和片段（像素）。

```c++
struct GouraudShader : public IShader {
    // ... 
}
```

🎸 **主舞台 main 函数**

最后，`main()` 函数，这是我们的主舞台。所有的预设、加载、渲染都在这里完成。

```c++
int main(int argc, char** argv) {
    //...
}

```

🎥 **Action！动作！**

1. **加载模型**: `new Model("../object/african_head/african_head.obj");` 这里，我们召唤了一个来自非洲的神秘头颅！
2. **视角设置**: 使用 `lookat`, `viewport`, 和 `projection` 函数，我们调整了观察点、视口和投影。这些都是电影导演级别的设置！
3. **初始化画布**: `TGAImage image (width, height, TGAImage::RGB);` 这里我们预备了一张画布，准备大展身手！
4. **渲染循环**: 嗯，这里有一个循环，负责画出那个非洲头颅。用了 `GouraudShader`，它会逐个面片地渲染模型。
5. **图片翻转和保存**: 最后，不要忘了翻转图像，并保存为 `.tga` 格式。现在你就可以拿这个图跟朋友炫耀了！

### 特别节目2之：细说GouraudShader

这个角色是渲染的灵魂，让我们细致入微地来看一下它的表演。

**🕺GouraudShader的组成**

- **变量：varying_intensity**

这个变量是一个3D向量（Vec3f类型），用来存储每个顶点的光照强度。这里的“varying”意味着这个变量会在顶点着色器和片段着色器之间“变化”（实际上是插值）。

- **方法：vertex**

这个函数负责处理每个顶点。它做了以下几件事：

1. **获取模型顶点**: `Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));` - 从3D模型中提取出一个顶点。
2. **坐标转换**: `gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;` - 使用各种矩阵变换将这个顶点从模型空间转换到屏幕空间。
3. **光照计算**: `varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir);` - 根据光照方向计算这个顶点的光照强度。

- **方法：fragment**

这个函数负责处理每个像素（片段）：

1. **插值计算**: `float intensity = varying_intensity*bar;` - 这里使用bar来进行插值，得到当前像素的光照强度。
2. **颜色设置**: `color = TGAColor(255, 255, 255)*intensity;` - 根据光照强度设置像素的颜色。
3. **像素保留**: `return false;` - 表示这个像素不会被丢弃，将出现在最终的图像中。

**🎭角色分析**

这个`GouraudShader`类扮演了一个全能艺人的角色：

- **化妆师**：通过`vertex`函数，对每个顶点进行"化妆"，也就是坐标变换和光照计算。
- **导演**：通过`fragment`函数，决定哪些像素应该用什么颜色来"演绎"，以及哪些像素应该被"剪辑"掉（这里没有剪辑，所有像素都保留）。
- **灯光师**：通过计算光照强度，控制场景的"明暗"，使得模型更加逼真。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921174128141.png" alt="image-20230921174128141" style="zoom:50%;" />

### 特别节目3之：开始绘画-片元着色器

OK，我们重新化妆一下，修改片元着色器（fragment）。

```c++
float intensity = varying_intensity*bar;// 通过插值计算得到当前像素的光照强度。
if (intensity>.85) intensity = 1;
else if (intensity>.60) intensity = .80;
else if (intensity>.45) intensity = .60;
else if (intensity>.30) intensity = .45;
else if (intensity>.15) intensity = .30;
else intensity = 0;
color = TGAColor(155, 155, 0)*intensity;
return false;
```

根据光照强度对像素进行了“分级”。每个级别都有一个特定的光照强度，就像你在照片编辑软件里手动设置不同级别的亮度。

这里将颜色设置为一个黄色（155, 155, 0），然后用上面的 `intensity` 来调节这个颜色。结果是一种不同深浅的黄色。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921174422284.png" alt="image-20230921174422284" style="zoom:50%;" />

着色器代码就像艺术家的调色板，你永远不知道接下来会画出什么样的图像！以下是一些有趣的着色器代码片段给大伙参考参考：

#### 🌈 彩虹着色器

```c++
float t = varying_intensity*bar;
color = TGAColor(
    128 + 127 * std::sin(t),
    128 + 127 * std::sin(t + 2.f/3.f * 3.14159f),
    128 + 127 * std::sin(t + 4.f/3.f * 3.14159f)
);
return false;
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921193615375.png" alt="image-20230921193615375" style="zoom:50%;" />

#### 📺 模拟老电视效果

```c++
float t = varying_intensity*bar;
float noise = rand() % 100 / 100.0;
if (noise > 0.9) {
    color = TGAColor(255, 255, 255);
} else if (noise < 0.1) {
    color = TGAColor(0, 0, 0);
} else {
    color = TGAColor(155, 155, 155) * t;
}
return false;
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921193810437.png" alt="image-20230921193810437" style="zoom:50%;" />

#### 🔥 火焰效果

```c++
float intensity = varying_intensity * bar;
color = TGAColor(
        255 * intensity,
        (int)(160 * std::sqrt(intensity)),
        0
);
return false;
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921193918227.png" alt="image-20230921193918227" style="zoom:50%;" />

#### 🌌 星空效果

```c++
float noise = rand() % 100 / 100.0;
if (noise > 0.98) {
    color = TGAColor(255, 255, 255);
} else {
    color = TGAColor(0, 0, 0);
}
return false;
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921193958547.png" alt="image-20230921193958547" style="zoom:50%;" />





## 6.4 升级Shader-支持UV纹理

在上一节我们把玩了`GouraudShader`，现在我介绍一个更强的选手，`Shader`，他不仅能够处理光照，还支持纹理贴图！

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921212841026.png" alt="image-20230921212841026" style="zoom:50%;" />

```c++
struct Shader : public IShader {
    Vec3f          varying_intensity; // written by vertex shader, read by fragment shader
    mat<2,3,float> varying_uv;        // same as above

    Vec4f vertex(int iface, int nthvert) override {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }

    bool fragment(Vec3f bar, TGAColor &color) override {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};
```

### 🎬 **Shader类的角色列表**

1. **varying_intensity**：这个角色没变，依然是顶点着色器计算出的光照强度。
2. **varying_uv**：新角色登场！这家伙用来存储每个顶点的纹理坐标（u,v）。

### 🎭 **vertex函数：多面手**

这个函数的流程与之前的大同小异，但多了一个关键步骤：

```c++
varying_uv.set_col(nthvert, model->uv(iface, nthvert));
```

这里，它从模型中获取每个顶点的纹理坐标（UV坐标）并存储下来。这些坐标将被用于后面的片段着色器中进行纹理贴图。

### 🌈 **fragment函数：艺术家**

在这个函数里，除了处理光照之外，我们还添加了纹理：

```c++
float intensity = varying_intensity*bar; // 同样是计算当前像素的光照强度
Vec2f uv = varying_uv*bar;               // 新功能：计算当前像素的纹理坐标
```

这里，`bar`用来进行插值，得到当前像素点的光照强度和纹理坐标。

```c++
color = model->diffuse(uv)*intensity; // 结合纹理和光照来计算最终颜色
```

接着，最精彩的部分来了。我们将之前计算的 `intensity` 和从 `uv` 贴图中获取的颜色相乘，得到的就是一个非常真实的颜色了。

### 🎨 **那么，这个Shader类都能做什么？**

1. **纹理贴图**：它能给3D模型穿上“衣服”，使模型看起来更逼真。
2. **漫反射光照**：它依然做好了基础的光照工作，让模型不会看起来像个平面。
3. **代码复用**：由于这个类继承了`IShader`，你可以很容易地在不同的渲染任务中复用这段代码。



## 6.5 学习法线贴图

法线贴图是一种用于3D计算机图形的技术，用于使3D模型的表面看起来更加详细，而无需使用更多的多边形。

简而言之，你使用一张纹理来存储有关如何微妙调整模型表面上的法线向量的信息，从而改变光与模型表面的相互作用方式。

### 第一关：纹理

用于法线贴图的纹理通常看起来像一种奇怪、抽象的蓝色混合物。每个像素的RGB值代表一个3D向量的X、Y、Z分量，这将用于在光照计算期间调整3D模型的表面法线。这与仅依赖模型几何形状计算得出的法线（每个顶点的法线）有所不同。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921214038676.png" alt="image-20230921214038676" style="zoom:50%;" />

### 第二关：全局坐标系与Darboux坐标系

- **全局坐标系**：在这里，法线是在全局（世界坐标）系统中表示的。

- **Darboux（切线空间）坐标系**：在这里，法线是相对于对象本身的表面表示的。Z向量垂直于物体的表面，而X和Y与物体的表面相切。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921222119442.png" alt="image-20230921222119442" style="zoom:50%;" />

通常，Darboux坐标系中的纹理看起来更加“有机”或“弯曲”，因为它是相对于物体表面的。全局坐标系中的纹理可能看起来更“统一”或“笔直”。因此，Darboux坐标系（切线空间）通常被认为更好，原因是它是对象相对的，使其在复杂的3D场景中具有更高的灵活性和通用性。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921222407082.png" alt="image-20230921222407082" style="zoom:50%;" />

```c++
struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
   }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        float intensity = std::max(0.f, n*l);
        color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};
[...]
    Shader shader;
    shader.uniform_M   =  Projection*ModelView;
    shader.uniform_MIT = (Projection*ModelView).invert_transpose();
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
```

### 第三关：经常见到的Uniform

首先，“Uniform”是GLSL（图形库着色器语言）中的一个保留关键字。这个关键字让你能够将常量（不会在渲染过程中改变的值）传递到着色器中。这里我们的渲染器也保留GLSL的名字。

就像给了演员一个剧本，告诉他们："别乱改，按这个演！"

### 第四关：光照计算

在上面这段代码中，光照强度的计算与之前基本相同，但有一个例外：它不是从每个顶点插值得到法线向量，而是从法线贴图纹理中获取这些信息。

换句话说，以前是依赖演员的自然演技（顶点法线），现在我们有了特效化妆师（法线贴图纹理）来让演员更出彩！

简而言之，上面这段代码就是将业余戏剧社升级到好莱坞级别的制作！

## 6.6 实现Phong模型

Phong模型包含了三项**漫反射 (Diffuse Reflection)**、**镜面反射 (Specular Reflection)**和**环境反射 (Ambient Reflection)**。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/agQhoxeqEs9IRpB.png" alt="image-20230525172542652" style="zoom:50%;" />
$$
\begin{aligned}
L & =L_a+L_d+L_s \\
& =k_a I_a+k_d\left(I / r^2\right) \max (0, \mathbf{n} \cdot \mathbf{l})+k_s\left(I / r^2\right) \max (0, \mathbf{n} \cdot \mathbf{h})^p
\end{aligned}
$$
<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230921234503261.png" alt="image-20230921234503261" style="zoom:50%;" />

```c++
struct Blinn_Phong_Shader : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    Vec4f vertex(int iface, int nthvert) override {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }
    bool fragment(Vec3f bar, TGAColor &color) override {
        Vec2f uv = varying_uv*bar;
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize(); // light direction
        Vec3f v = Vec3f(0, 0, -1); // simplified view direction
        Vec3f h = (l + v).normalize(); // halfway vector

        float spec = pow(std::max(0.f, n*h), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);
        return false;
    }
};
```

接下来，我们将会讨论阴影。

## 7.1 阴影

需要注意的是，这里我们谈论的是硬阴影，软阴影的实现又是另外一回事了。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230922113248506.png" alt="image-20230922113248506" style="zoom: 33%;" /><img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230922113300470.png" alt="image-20230922113300470" style="zoom: 33%;" />

上面两张图片就是本章要实现的内容。读者可能会想，右边的图片拍摄角度是不是有问题。实际上，右边这张图的拍摄位置是光源所在的位置。至于为什么，我们就在本章详细探讨。你可能会看到上图有一些瑕疵，这正是 [Z-fighting](https://en.wikipedia.org/wiki/Z-fighting) 现象。

### 第一关：目前的问题

回到我们上一章完成的进度。根据我们的常识，在光线照射不到的地方（图中高亮人物脖子的一侧），应该与能照射到的地方有比较明显的光照分界。目前我们的渲染器输出的效果是左图，而正常来说应该像右边的图片那样。

<img src="/Users/remooo/Library/Application%20Support/typora-user-images/image-20230922151625557.png" alt="image-20230922151625557" style="zoom: 33%;" /><img src="/Users/remooo/Library/Application%20Support/typora-user-images/image-20230922152552309.png" alt="image-20230922152552309" style="zoom: 33%;" />

为了解决这个问题，就要搬出图形学大名鼎鼎的 two-pass 方法（two-pass rendering）了。这个方法基本思想是先从光源处渲染一副有深度信息的图片，这张照片记录了从光源视角看到的深度信息。接下来再从主相机视角渲染图像，通过上一 Pass 的深度信息判断当前的渲染像素点时候直接被光照射。

### 第二关：第一趟渲染-从光源出发

```c++
{ // rendering the shadow buffer
    TGAImage depth(width, height, TGAImage::RGB);
    lookat(light_dir, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(0);

    DepthShader depthshader;
    Vec4f screen_coords[3];
    for (int i=0; i<model->nfaces(); i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = depthshader.vertex(i, j);
        }
        triangle(screen_coords, depthshader, depth, shadowbuffer);
    }
    depth.flip_vertically(); // to place the origin in the bottom left corner of the image
    depth.write_tga_file("depth.tga");
}
```

<img src="/Users/remooo/Library/Application%20Support/typora-user-images/image-20230922163751926.png" alt="image-20230922163751926" style="zoom:50%;" />

### 第三关：第二趟渲染-从主相机出发

```c++
{ // rendering the frame buffer
    TGAImage frame(width, height, TGAImage::RGB);
    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());

    Shader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(Viewport*Projection*ModelView).invert());
    Vec4f screen_coords[3];
    for (int i=0; i<model->nfaces(); i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, frame, zbuffer);
    }
    frame.flip_vertically(); // to place the origin in the bottom left corner of the image
    frame.write_tga_file("framebuffer.tga");
}
```

效果非常好～该有的阴影都有了。但是我们注意这只怪物的手，阴影非常奇怪。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230922163743152.png" alt="image-20230922163743152" style="zoom:50%;" />

奇怪的手臂阴影，这种现象我们称为阴影痤疮（Shadow Acne）。当渲染的物体与其阴影深度映射几乎重合时，可能会出现阴影斑点或噪声。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230922164007209.png" alt="image-20230922164007209" style="zoom:50%;" />

怎么解决呢？可以考虑提高“阈值”，让物体没那么容易被相邻的部位遮挡住自己。具体到写代码上，就是稍微减小对应点的深度值。一点点小小的魔法，就可以解决这个烦人的问题了。

```c++
float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]+43.34); // magic coeff to avoid z-fighting
```

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230922170825404.png" alt="image-20230922170825404" style="zoom:50%;" />

项目的代码可以还是继续提供给读者们解读，[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/7_1_Shadow)。

由于我比较懒，不想写太详细解读了，读者自己应该可以读懂。

## 8.1 环境光遮蔽 - 模拟全局光照效果

上一讲我们实现了 Phong 光照模型，他的组成有三项分别是环境光、高光以及漫反射。还讲了一种渲染策略，Two-Pass渲染。这里，我们介绍一种新的全局光照模拟技术，环境光遮蔽（Ambient Occlusion, AO）。

但是，Phong模型只考虑了物体与特定光源之间的直接互动。在物体的小凹陷或接近的物体之间的接触区域，经常会出现微小的阴影。这些阴影往往与任何特定的光源无关，而是由于环境光被周围的几何体部分遮挡所造成的。Phong模型无法捕获这种效果，而AO可以。

巧合的是，AO并不直接依赖于场景中的光源位置或属性。这使得它可以与任何光照模型（如Phong模型）结合使用，并为渲染效果增添额外的真实感。

### 特别节目：知识脉络

读者读到这里可能会有很多疑惑，对这些名词概念的层级把握不清楚，这里给读者梳理一下。

1. **光照模型**
   - **Phong模型**
     - 环境光
     - 漫反射光
     - 镜面反射光
   - **Blinn-Phong模型**
   - Lambert模型
   - Cook-Torrance模型
   - Oren-Nayar模型
2. **全局光照模拟技术**
   - **环境光遮蔽 (AO)**
   - 光线追踪 (Ray Tracing)
   - 光子映射 (Photon Mapping)
   - 辐射度缓存 (Radiance Caching)
   - Final Gathering
3. **渲染策略**
   - **Two-Pass渲染**
     - Two-Pass阴影映射
   - 多Pass渲染
   - 延迟渲染 (Deferred Rendering)
   - 前向渲染 (Forward Rendering)
4. **后处理效果**
   - 色调映射 (Tone Mapping)
   - 抗锯齿技术 (如 MSAA, FXAA, TAA)
   - 深度模糊 (Depth of Field)
   - 动态模糊 (Motion Blur)
5. **纹理技术**
   - 传统纹理映射
   - 法线贴图 (Normal Mapping)
   - 抛物线映射 (Parallax Mapping)
   - 物理基础渲染 (Physically-Based Rendering, PBR) 的材质纹理（如 Albedo, Roughness, Metallic）



### 第一关：啥是AO？如何结合Phong使用？

环境光遮蔽的基本思想是评估一个给定的表面点在多大程度上被其周围的几何体遮挡。一个被其他物体严重遮挡的点会接收到更少的环境光，因此看起来会更暗。

当你在场景中使用Phong模型和环境光遮蔽时，通常的方法是先计算Phong模型的环境反射组成部分，然后使用环境光遮蔽来调整这个值。具体来说，你会将环境光遮蔽值乘以Phong模型的环境光分量，从而在需要的地方减少环境光。

计算方式有很多，最简单的是屏幕空间技术（如SSAO，Screen Space Ambient Occlusion）。但是在介绍这个方法之前，我们不妨先自行思考一下我们如何实现。

### 第二关：做梦

想象一下你正在拍摄一个物体，而这个物体上方有一个半透明的伞，伞的下半部分可以发出均匀的光。现在，为了知道物体的哪些部分更容易被这个光照亮，你决定在伞的内侧随机选择一些点，然后看看从这些点发出的光线能不能照到物体。

它采用一种“暴力”的方法：随机选择很多点，并从每一个点观察物体。

为了记录物体上哪些部分被光照到了，我们用一个图片来记录。每一次从伞的一个点看物体，都会产生一个新的图片。

最后，我们把所有的图片混合在一起，得到一个平均的图片。这个图片会告诉我们，物体的哪些部分通常更容易被光照到。

但是，这种方法也有缺点。比如，如果物体的两个手臂在最终的图片中使用了相同的位置，那么这两个手臂上的光就会被计算两次，这会导致最终的效果不准确。

### 第三关：屏幕空间环境遮挡 (SSAO) 

全局照明非常昂贵，需要为很多点计算可见性。为了找到一个在计算时间和渲染质量之间的平衡，我们尝试使用SSAO。

在这里我们将SSAO用作一个单独的效果，只计算环境遮挡而不计算其他光照。

- ZShader

这个着色器主要用于渲染z-buffer，只关心深度，不关心颜色。

```c++
struct ZShader : public IShader {
    mat<4,3,float> varying_tri;

    Vec4f vertex(int iface, int nthvert) override {
        Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) override {
        color = TGAColor(0, 0, 0);
        return false;
    }
};
```

- Max_elevation_angle

估算一个像素点与其周围环境的最大仰角，这是评估遮挡程度的关键函数。

```c++
float max_elevation_angle(float *zbuffer, Vec2f p, Vec2f dir) {
    float maxangle = 0;
    for (float t=0.; t<1000.; t+=1.) {
        Vec2f cur = p + dir*t;
        if (cur.x>=width || cur.y>=height || cur.x<0 || cur.y<0) return maxangle;
        float distance = (p-cur).norm();
        if (distance < 1.f) continue;
        float elevation = zbuffer[int(cur.x)+int(cur.y)*width]-zbuffer[int(p.x)+int(p.y)*width];
        maxangle = std::max(maxangle, atanf(elevation/distance));
    }
    return maxangle;
}
```

- 环境光遮蔽的计算

对于每个像素，使用8个方向的射线来评估其环境遮挡程度。

```c++
for (int x=0; x<width; x++) {
    for (int y=0; y<height; y++) {
        if (zbuffer[x+y*width] < -1e5) continue;
        float total = 0;
        for (float a=0; a<M_PI*2-1e-4; a += M_PI/4) {
            total += M_PI/2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
        }
        total /= (M_PI/2)*8;
        total = pow(total, 100.f);
        frame.set(x, y, TGAColor(total*255, total*255, total*255));
    }
}
```

项目完整代码，[链接🔗](https://github.com/Remyuu/Tiny-Renderer/tree/8_1_AO)。

<img src="https://regz-1258735137.cos.ap-guangzhou.myqcloud.com/remo_t/image-20230925172916074.png" alt="image-20230925172916074" style="zoom:50%;" />







## 附录1. c++模版类 - 从入门到入土

第一关：为什么需要模版类？

第二关：「函数模版」

第三关：「类模版」

第四关：「多模板参数」与「非类型参数」

第五关：「模板特化」

第六关：「类型推断」

​	1. auto & decltype 2. 模板中的基本类型推断3. 自动构造模版类型4. 尾返回类型

第七关：「变量模板」

第八关：「模板类型别名」

第九关：模板的SFINAE原则

第十关：模板与友元

第十一关：折叠表达式

第十二关：模板概念 - C++20

第十三关： `std::enable_if` 和 SFINAE

第十四关：类模板偏特化

第十五关：`constexpr` 和模板

第十六关：模板中的嵌套类型

第十七关：模板参数包与展开

第十八关：Lambda 表达式与模板

第十九关：模板递归

第二十关：带有模板的继承

### 第一关：为什么需要模版类？

在没有模板之前，如果你想为不同的数据类型编写相同的功能，你可能需要为每种数据类型写一个函数或类。这会导致大量的重复代码。

> 用专业的话来说就是，函数模板和类模板在 C++ 中是用来支持泛型编程的工具。泛型编程是一种编写与类型无关的代码的方法。这就意味着，通过使用模板，你可以创建一个能够适应任何数据类型的函数或类，而不需要为每种数据类型都重新编写代码。

例如一个函数，它的任务是交换两个整数的值。后来，你又想交换两个浮点数。没有模板，你可能需要为每种数据类型编写单独的函数。

### 第二关：「函数模版」

解决上面提到的问题，非常简单。

```c++
// 模版函数
template <typename T>
void swap(T &a, T &b) {
    T temp = a;
    a = b;
    b = temp;
}
// 调用方法
int x = 5, y = 10;
swap(x, y);

double m = 5.5, n = 10.5;
swap(m, n);
```

`template <typename T>` 声明了一个模板函数。此处的 `T` 可以被认为是一个占位符，它在编译时会被实际的数据类型替换。

### 第三关：「类模版」

类模版跟函数模版差不多。下面的例子是一个用于存储任意类型的数组的类。

```c++
// 模版类
template <typename T>
class Array {
private:
    T *data;
    int size;
public:
    Array(int s) : size(s) {
        data = new T[size];
    }
    ~Array() {
        delete[] data;
    }
    T& operator[](int index) { // 实现索引获取元素
        return data[index];
    }
};
// 如何调用？
Array<int> intArray(10);
Array<double> doubleArray(10);
```

### 第四关：「多模板参数」与「非类型参数」

可以为一个模板定义多个参数。同时，参数可以是上面所说的 `typename T` 非类型参数，也可以是类型参数，像下面代码中的 `int SIZE` 。

```c++
// 多模板参数、非类型参数
template <typename T, int SIZE>
class FixedArray {
private:
    T data[SIZE];
public:
    T& operator[](int index) {
        return data[index];
    }
}
// 使用方式
FixedArray<int, 10> intArray;
```

### 第五关：「模板特化」

有时候，希望某个模板对某个特定类型有一个不同的实现。这时你可以使用模板特化。假如现在有下面的模版。

```c++
template <typename T>
class Printer {
public:
    void print(T value) {
        std::cout << "General print: " << value << std::endl;
    }
};
```

但我希望对于 `int` 类型有一个特殊的输出。

```c++
template <>
class Printer<int> {
public:
    void print(int value) {
        std::cout << "Special print for int: " << value << std::endl;
    }
};
```

### 第六关：「类型推断」

#### 1. auto & decltype

在 C++11 中引入了很多特性，其中一个与类型推断相关的特性是“auto”关键字。除了刚才说的“auto”，C++11还引入了“decltype”关键词，可以判断一个表达式的类型。

```c++
auto x = 42;    // x 的类型被推断为 int
auto y = 3.14;  // y 的类型被推断为 double

int num = 5;
decltype(num) y = 10;  // y 的类型被推断为 int
```

#### 2. 模板中的基本类型推断

此外，函数模板的类型推断在 C++ 中已经存在了一段时间，但 C++11 增强了这一特性。函数模板可以自动推断类型参数。

```c++
template <typename T>
void show(T value) {
    std::cout << value << std::endl;
}

// 调用
show(5);        // 5
show(3.14);     // 3.14
```

#### 3. 自动构造模版类型

在 C++17 之后，类型推断就更加强大了。在 C++17 之前，类模板的类型参数不能自动推断。但是从 C++17 开始，我们可以通过模板参数的自动类型推断来构造类模板的对象。

```c++
template <typename T>
class MyClass {
    T data;
public:
    MyClass(T d) : data(d) {}
    void display() {
        std::cout << data << std::endl;
    }
};

int main() {
    // C++17 之前的方式
    MyClass<int> obj1(10);
    obj1.display();

    // C++17 之后的方式
    MyClass obj2(10);    // 自动推断为 MyClass<int>
    obj2.display();
}
```

#### 4. 尾返回类型

C++11 引入了尾返回类型，使得函数的返回类型可以基于其参数进行推断，这对于模板特别有用。下面代码的 `->` 用于指定函数的尾返回类型。此时，`auto` 告诉编译器函数返回类型将由其后的表达式来决定，也就是刚刚说的 `->` 。

```c++
template <typename T1, typename T2>
auto add(T1 x, T2 y) -> decltype(x + y) {
    return x + y;
}

int main() {
    auto result = add(5, 3.14);  // 结果的类型推断为 double
    std::cout << result << std::endl;
}
```

### 第七关：「变量模板」

C++14 引入了变量模板，它允许你为模板定义静态数据成员。它与函数和类的模板类似，但是用于变量。

我们定义了一个名为 `pi` 的变量模板，它为每种类型 `T` 提供了 π 的近似值。你可以像使用其他模板那样使用变量模板，但需要指定模板参数来获取相应的变量实例。

```c++
template <typename T>
constexpr T pi = T(3.1415926535897932385);

int main() {
    std::cout << pi<int> << std::endl;        // 输出 3
    std::cout << pi<double> << std::endl;     // 输出 3.14159...
}
```

一般这个「变量模版」非常适用于那些需要为不同类型提供不同值或配置的情况。同时使用的时候注意以下事项：

- 变量模板通常与 `constexpr` 一起使用，以确保它们在编译时是常数。
- 变量模板的实例化方式与函数或类模板相似。当你第一次为特定的类型使用变量模板时，编译器将为该类型创建一个实例。

###  第八关：「模板类型别名」

「模板类型别名」为已存在的模板类型定义了一个新的、更简短的名称。

在 C++11 之前，如果你想为复杂的模板类型创建别名，这往往是非常麻烦的。C++11 引入了 `using` 关键字来创建模板类型别名，这提供了一个更清晰、更简洁的方式来定义这些别名。

这里以 第三关 的例子说明创建别名的最简单实践。

```c++
template <typename T>
using MyArray = Array<T>;
```

这里再举一个简单、常用的例子为常见的向量类型提供别名。

```c++
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

Vec3f position;
```

值得注意的是，你也可以用old school的方法，即typedef。上下两段代码是完全一致的。

```c++
typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
```

他们的区别在于，`typedef` 使用旧的 C/C++ 语法，而`using` 是 C++11 引入的新语法，用于定义类型别名。对于简单的类型别名，这两种方法之间的差异可能不明显。但是，当涉及到更复杂的类型，如函数指针或模板类型，`using` 的语法往往更为简洁和直观。

> 这里拓展一下，`using` 和 `typedef`  两者一个主要的区别是，`using`可以为模板提供别名。
>
> ```c++
> template <typename T>
> using Vec2Ptr = Vec2<T>*;
> ```

### 第九关：模板的SFINAE原则

SFINAE 原则是 C++ 模板中的一个特性。SFINAE是“Substitution Failure Is Not An Error”（替换失败不是错误）的缩写。当试图用给定的模板参数替换模板时，如果发生错误，则该特殊化不被考虑。

想象一下你正在为一个魔法展示准备一套卡片。每张卡片上都有一个指令，例如“变成兔子”或“飞起来”。但有一张卡片的指令是“让猪飞起来”。显然，这是一个不可能的任务。

在通常情况下，魔术师会看到这张卡片并说：“这个指令有问题，展示失败了！”。但在 SFINAE 的世界里，魔术师会说：“好吧，这张卡片不工作，让我试试下一张”。

换句话说，SFINAE 就像是编译器的一个内置魔术师。当你尝试用一个不合适的类型进行模板替换时，而不是直接报错，编译器会悄悄地“忽略”那个模板，并尝试其他的选项。

直到没有选项合适（**No matching**）或者很多合适选项（**Ambiguous**），编译器就会报出错误。

一个简单的场景：我们希望写一个函数 `printValue`，该函数可以打印整数或字符串。但是，如果我们尝试使用其他类型，这个函数就不应该存在。

```c++
#include <iostream>
#include <type_traits>

// 1. 对于整数类型
template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type
printValue(const T& val) {
    std::cout << "Integer: " << val << std::endl;
}

// 2. 对于字符串类型
template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value>::type
printValue(const T& val) {
    std::cout << "String: " << val << std::endl;
}

int main() {
    printValue(42);           // 输出: Integer: 42
    printValue(std::string("Hello")); // 输出: String: Hello
    
    // printValue(3.14);      // 这一行会引起编译错误，因为没有适合double类型的printValue版本
    return 0;
}
```

这一长串代码确实有点丑陋了，我们将代码拆开详细看看。

```c++
// 节选自上面的代码
template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type
printValue(const T& val) {
    std::cout << "Integer: " << val << std::endl;
}
```

1. **模板声明**:

   ```cpp
   template <typename T>
   ```

   声明了一个模板函数，其中 `T` 是一个待定的类型。你可以为 `T` 提供任何类型，比如 `int`、`double`、`std::string` 等，但是函数的实际行为取决于你提供的类型。

2. **返回类型**:

   ```cpp
   typename std::enable_if<std::is_integral<T>::value>::type
   ```

   这段代码使用了两个主要的模板工具：`std::enable_if` 和 `std::is_integral`。

   - `std::is_integral<T>::value` 是一个类型特性，检查 `T` 是否是整数类型。如果是，它返回 `true`；否则返回 `false`。

   - `std::enable_if` 是一个模板，它有一个嵌套的 `type` 成员，但这个成员只在给定的布尔表达式为 `true` 时存在。在这里，它检查前面的 `std::is_integral<T>::value` 是否为 `true`。

   结合起来，这意味着：

   - 如果 `T` 是整数类型，函数的返回类型将是 `void`（因为 `std::enable_if` 的默认类型是 `void`）。
   - 如果 `T` 不是整数类型，由于 `type` 成员不存在，SFINAE 将阻止此函数模板被实例化，因此该版本的 `printValue` 函数将不可用。

如果我想让当函数传入int类型时输出double类型，可以这样做：

```c++
template <typename T>
typename std::enable_if<std::is_same<T, int>::value, double>::type
printValue(const T& val) {
    std::cout << "Integer: " << val << std::endl;
    return static_cast<double>(val);
}

int main() {
    double result = printValue(42);
    std::cout << "Returned value: " << result << std::endl;
    // print 42.0
}
```

关键部分是 `typename std::enable_if<std::is_same<T, int>::value, double>::type`，这会检查 `T` 是否与 `int` 相同。如果是，它将产生类型 `double`。如果不是，该版本的 `printValue` 函数将由于 SFINAE 而不被考虑。

有朋友可能会说，为什么不用多态呢？写这坨代码实在是太难看了，我用多态写那叫一个简洁：

```c++
double printValue(int val) {
    std::cout << "Integer: " << val << std::endl;
    return static_cast<double>(val);
}

void printValue(double val) {
    std::cout << "Double: " << val << std::endl;
}
```

以下是一些常见的解释：

1. **泛型编程**: 使用模板，你可以为各种类型编写通用的代码，而不仅仅是那些你预先知道的类型。
2. **类型约束**: 通过 SFINAE 和其他模板技巧，你可以对哪些类型可以用于你的泛型代码施加更精细的约束。例如，你可能想要一个函数，它只接受具有某些成员函数的对象。
3. **编译时优化**: 由于模板在编译时实例化，编译器可以为每个特定的类型生成优化过的代码，这可能会导致更高的执行效率。
4. **灵活性**: 模板提供了更多的灵活性，例如模板元编程、模板特化等，允许更复杂和高效的编程技术。
5. **类型透明性**: 当使用模板时，原始类型信息在使用模板函数或类的地方保持不变。这与多态不同，其中类型信息可能会丢失，特别是在使用继承和虚函数时。

随着进一步学习以及项目的接触，我们可以更加体会到这种编程方式的优缺点。

### 第十关：模板与友元

模板类或函数可以声明为另一个类或函数的友元。

```c++
template <typename T>
class Container {
private:
    T data;
public:
    Container(T d) : data(d) {}
    template <typename U>
    friend bool operator==(const Container<U>&, const Container<U>&);
};

template <typename T>
bool operator==(const Container<T>& lhs, const Container<T>& rhs) {
    return lhs.data == rhs.data;
}
```

### 第十一关：折叠表达式

C++17中的折叠表达式可以简化某些变长模板参数的操作。

例如，要计算所有给定参数的总和：

```c++
template<typename... Args>
auto sum(Args... args) {
    return (... + args);
}
```

### 第十二关：模板概念(Concepts) - C++20

C++20引入了模板的概念，允许你为模板参数指定更明确的约束。只有满足给定概念的类型才可以作为`print`函数的参数。

比如说，

```c++
template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <Arithmetic T>
T add(T a, T b) {
    return a + b;
}
```

这里是其他的一些特性：

- `std::is_integral<T>`：检查 `T` 是否是整数类型。
- `std::is_floating_point<T>`：检查 `T` 是否是浮点数类型。
- `std::is_array<T>`：检查 `T` 是否是数组。
- `std::is_pointer<T>`：检查 `T` 是否是指针。
- `std::is_reference<T>`：检查 `T` 是否是引用。
- `std::is_class<T>`：检查 `T` 是否是类或结构体类型。
- `std::is_function<T>`：检查 `T` 是否是函数。

另外还可以通过其他方法检查“一个类型是否可以被输出流输出”。也就是在下面代码中，我们定义了一个`Printable`的conecpt，要满足这个概念，类型 `T` 必须满足 `requires` 表达式中的要求。

```c++
template <typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

template <Printable T>
void print(T value) {
    std::cout << value;
}
```

其中， `requires` 表达式是与**概念** (concepts) 相关的一种新特性，用于描述一个类型必须满足的要求。

```c++
requires ( 参数 ) { 要求列表 }
```

在这里，我们要求类型 `T` 必须支持一个操作，即：当你尝试将 `t` 输出到 `std::cout` 时，结果的类型必须是 `std::ostream&`。在 `requires` 表达式中，`->` 符号被用于指定一个表达式的预期返回类型。

另外注意，`requires` 表达式是在编译阶段处理的。

###  第十三关： `std::enable_if` 和 SFINAE

上面我们已经有所提及，当我们希望根据某种条件来决定是否生成模板函数或类时，`std::enable_if`非常有用。

例如，假设你有一个函数，你只希望当传入的类型是整数时，它才存在：

```c++
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
functionOnlyForIntegers(T value) {
    return value * 2;
}
```

### 第十四关：类模板偏特化

现在我们从头开始梳理一遍类模板。假设我们有以下基本模板。

```c++
template <typename T1, typename T2>
class MyPair {
    T1 first;
    T2 second;
    // ... 其他成员函数 ...
};
```

接下来，对类模板偏特化。假设我们想为第二个模板参数是指针类型的所有情况提供特化。这里的"偏"意味着我们不是为两个特定的类型提供特化，而是只为一个类型（这里是 T2）提供。

```c++
template <typename T1, typename T2>
class MyPair<T1, T2*> {
    T1 first;
    T2* second;
    // ... 其他成员函数 ...
};
```

需要注意的是，函数模板不支持偏特化，但可以通过重载来达到类似的效果。

### 第十五关：`constexpr` 和模板

`constexpr` 是 C++11 引入的关键字，它用于声明常量表达式，这些表达式在编译时就可以计算出结果。使用`constexpr`与模板一起可以在编译时生成高效的代码。譬如下面的例子。

```c++
constexpr int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}
constexpr int value = factorial(5);  // 120
```

那么结合 `constexpr` 和模板的例子是啥样的？当 `constexpr` 与模板结合使用时，你可以为各种类型创建编译时函数或实体，它们将针对给定的类型进行优化，并在编译时生成结果。

```c++
template <typename T>
constexpr T square(const T& value) {
    return value * value;
}

constexpr int int_val = square(5);        // 25
constexpr double double_val = square(5.0); // 25.0
```

两者结合的优势很大，我这里列出两点：

- **性能**：结合 `constexpr` 和模板，生成的代码是在编译时优化的，这可以消除运行时计算的需要，从而提高性能。
- **泛型**：模板使你可以为多种类型编写代码，而 `constexpr` 确保了对每种类型的高效实现。

### 第十六关：模板中的嵌套类型

一个模板可以在其内部定义另一个模板类：

```c++
template <typename T>
class Outer {
    T data;
public:
    template <typename U>
    class Inner {
        U innerData;
    };
};
```

接下来，让我们给 `Outer` 和 `Inner` 类添加一些成员函数，使它们更具功能性。

```c++
template <typename T>
class Outer {
    T data;
public:
    Outer(T d) : data(d) {}

    T getOuterData() const { return data; }

    template <typename U>
    class Inner {
        U innerData;
    public:
        Inner(U d) : innerData(d) {}

        U getInnerData() const { return innerData; }
    };
};
```

使用示例：

```c++
Outer<int> outerInstance(10);
std::cout << "Outer data: " << outerInstance.getOuterData() << std::endl; 
// Outputs: Outer data: 10

Outer<int>::Inner<double> innerInstance(5.5);
std::cout << "Inner data: " << innerInstance.getInnerData() << std::endl; 
// Outputs: Inner data: 5.5
```

进一步添加功能，在 `Outer` 类中定义一个函数，该函数接受一个 `Inner` 对象并与之交互。

```c++
template <typename T>
class Outer {
    T data;
public:
    Outer(T d) : data(d) {}

    T getOuterData() const { return data; }
    
	// ---- template ----
    template <typename U>
    class Inner {
        U innerData;
    public:
        Inner(U d) : innerData(d) {}

        U getInnerData() const { return innerData; }
    };
    // ---- -------- ----
    
    template <typename U>
    void printCombinedData(const Inner<U>& inner) {
        std::cout << "Combined data: " << data << " and " << inner.getInnerData() << std::endl;
    }
};

// 使用：
Outer<int> outerInstance(10);
Outer<int>::Inner<double> innerInstance(5.5);
outerInstance.printCombinedData(innerInstance);  // Outputs: Combined data: 10 and 5.5
```

总之需要知道，外部类完全可以访问其内部类及其成员，但它需要拥有内部类的对象实例才能访问内部类的非静态成员。

### 第十七关：模板参数包与展开

当使用变长模板参数时，你可以使用模板参数包。使用`...`修饰的参数被称为参数包。

```c++
template <typename... Args>
void printValues(Args... args) {
    (std::cout << ... << args);  // 展开参数
}

int main() {
    printValues(1, 2, 3, "hello", 'c');
    //Same As ： std::cout << 1 << 2 << 3 << "hello" << 'c';
}
```

如果要用多态来实现上面的效果，将会变得比较复杂。需要为每一种要输出的类型创建一个公共的基类并实现虚函数。然后为每种具体的类型实现一个子类。下面是用多态来实现的，可以看出模版参数包的优越性了吧。

```c++
#include <iostream>
#include <vector>

class Printable {
public:
    virtual ~Printable() {}
    virtual void print() const = 0;
};

class PrintInt : public Printable {
    int value;
public:
    PrintInt(int v) : value(v) {}
    void print() const override {
        std::cout << value;
    }
};

class PrintString : public Printable {
    std::string value;
public:
    PrintString(const std::string& v) : value(v) {}
    void print() const override {
        std::cout << value;
    }
};

class PrintChar : public Printable {
    char value;
public:
    PrintChar(char v) : value(v) {}
    void print() const override {
        std::cout << value;
    }
};

void printValues(const std::vector<Printable*>& values) {
    for (const auto& val : values) {
        val->print();
    }
}

int main() {
    std::vector<Printable*> values = {new PrintInt(1), new PrintInt(2), new PrintInt(3), new PrintString("hello"), new PrintChar('c')};
    printValues(values);

    // Cleaning up
    for (auto ptr : values) {
        delete ptr;
    }
}
```

还记得 十一关 讲解的折叠表达式吗？折叠表达式是 C++17 引入的，是一种新的、更简洁的方式来展开参数包，并对其应用特定的运算。在 C++17 之前，当需要在模板中使用参数包的时候，通常需要使用某种机制对其进行展开。在 C++11 和 C++14 中，展开参数包通常涉及到递归的模板技巧。例如，

```c++
template <typename T>
void printValues(T value) {
    std::cout << value << std::endl;
}
template <typename First, typename... Rest>
void printValues(First first, Rest... rest) {
    std::cout << first << ", ";
    printValues(rest...);  // 展开剩余的参数
}
// 使用
int main() {
    printValues(1, 2, 3);        // 输出: 1, 2, 3
    printValues("a", "b", "c");  // 输出: a, b, c
    return 0;
}
```

而使用了折叠表达式，就不用涉及递归输出了，上下两则代码完全一致。

``` c++
template <typename... Args>
void printValues(Args... args) {
    (std::cout << ... << args);
}
// 使用
int main() {
    printValues(1, 2, 3, "hello", 'c');  // 输出：123helloc
    return 0;
}
```

### 第十八关：Lambda 表达式与模板

```c++
auto lambda = []<typename T>(T value) { return value * 2; };
auto result = lambda(5);  // result为10
```

进一步添加“概念”，以确保类型是可计算的。这里直接使用了`std::is_arithmetic_v`。

```c++
#include <iostream>
#include <type_traits>

int main() {
    auto genericLambda = [](auto x) {
        static_assert(std::is_arithmetic_v<decltype(x)>, "Type must be arithmetic!");
        return x * x;
    };

    std::cout << genericLambda(5) << std::endl;    // 输出：25
    std::cout << genericLambda(5.5) << std::endl;  // 输出：30.25

    // genericLambda("hello"); // 编译错误：Type must be arithmetic!
}
```

### 第十九关：模板递归

模板递归是一种非常强大的技巧，但也需要谨慎使用，因为它可能导致编译时间增加和代码膨胀。

在前面我们已经见识到了模版的强大。例如，计算阶乘或斐波那契数列，直接在编译期间就可以完成计算，减少运行时的计算量。

```c++
constexpr int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}
constexpr int value = factorial(5);  // 120
```

### 第二十关：带有模板的继承

类模板可以继承自其他类模板。下面是一个最简单的例子，我们逐渐完善他。

```c++
template <typename T>
class Base {};

template <typename T>
class Derived : public Base<T> {};
```

#### 1. 模版基类

可以创建一个模板基类，使得不同的子类可以以不同的方式特化或使用这个基类。

```c++
template <typename T>
class Base {
public:
    T value;
    Base(T val) : value(val) {}
    void show() { std::cout << value << std::endl; }
};

class Derived : public Base<int> {
public:
    Derived(int v) : Base(v) {}
    void display() { std::cout << "Derived: " << value << std::endl; }
};

int main() {
    Derived d(10);
    d.show();
    d.display();
}
```

#### 2. 模版子类

可以使子类是模板，而基类不是。这样，就可以为基类定义一组行为，而子类则为这些行为提供具体的实现。

```c++
class Base {
public:
    virtual void show() const = 0;
};

template <typename T>
class Derived : public Base {
    T value;
public:
    Derived(T v) : value(v) {}
    void show() const override {
        std::cout << "Value: " << value << std::endl;
    }
};

int main() {
    Derived<int> d1(5);
    Derived<double> d2(3.14);
    d1.show();
    d2.show();
}
```



#### 3. 在模板类中继承模板基类

子类和基类都可以是模板，这样你可以创建高度灵活和可重用的设计。

```c++
template <typename T>
class Base {
public:
    T value;
    Base(T val) : value(val) {}
    virtual void show() const {
        std::cout << "Base: " << value << std::endl;
    }
};

template <typename T>
class Derived : public Base<T> {
public:
    Derived(T v) : Base<T>(v) {}
    void show() const override {
        std::cout << "Derived: " << this->value << std::endl;
    }
};

int main() {
    Derived<int> d(10);
    d.show();
}
```

### 第二十一关：`std::type_trait`的工具集

`<type_traits>`头文件提供了一组用于类型检查和修改的模板，可以在编译时获取和操作类型的信息。

```c++
static_assert(std::is_same<std::remove_const<const int>::type, int>::value);
```

以下是 `std::type_traits` 中一些常用的工具：

1. **基础类型检查**:
   - `std::is_integral<T>`: 检查T是否是一个整数类型。
   - `std::is_floating_point<T>`: 检查T是否是一个浮点类型。
   - `std::is_arithmetic<T>`: 检查T是否是算术类型（整数或浮点数）。
   - `std::is_pointer<T>`: 检查T是否是指针。
   - `std::is_reference<T>`: 检查T是否是引用。
   - `std::is_array<T>`: 检查T是否是数组。
   - `std::is_enum<T>`: 检查T是否是枚举类型。

2. **类型关系检查**:
   - `std::is_same<T, U>`: 检查两个类型是否完全相同。
   - `std::is_base_of<Base, Derived>`: 检查`Base`是否是`Derived`的基类。
   - `std::is_convertible<T, U>`: 检查类型T是否可以被隐式转换为U。

3. **类型修改器**:
   - `std::remove_reference<T>`: 去除引用，得到裸类型。
   - `std::add_pointer<T>`: 为类型T添加一个指针。
   - `std::remove_pointer<T>`: 去除指针。
   - `std::remove_const<T>`: 去除常量限定符。
   - `std::add_const<T>`: 添加常量限定符。

4. **其他**:
   - `std::underlying_type<T>`: 对于枚举类型T，得到对应的底层类型。
   - `std::result_of<F(Args...)>`: 对于函数类型F，返回它使用参数`Args...`调用时的返回类型。

5. **辅助类型**:
   - 对于上述的每个特性检查，都有一个对应的`_v`后缀的变量模板，如`std::is_integral_v<T>`，它直接返回bool值，这使得代码更简洁。

```c++
static_assert(std::is_same<std::remove_const<const int>::type, int>::value);
static_assert(std::is_integral_v<int>);
```

### 第二十二关：模板与动态多态性

尽管模板提供了一种静态多态性形式，但它们也可以与虚函数和动态多态性结合使用。

```c++
#include <iostream>
#include <vector>

class Base {
public:
    virtual void print() const {
        std::cout << "Base class." << std::endl;
    }

    virtual ~Base() {}
};

class Derived1 : public Base {
public:
    void print() const override {
        std::cout << "Derived1 class." << std::endl;
    }
};

class Derived2 : public Base {
public:
    void print() const override {
        std::cout << "Derived2 class." << std::endl;
    }
};

template <typename T>
class Container {
private:
    std::vector<T*> elements;

public:
    void add(T* elem) {
        elements.push_back(elem);
    }

    void printAll() const {
        for (auto& elem : elements) {
            elem->print();
        }
    }
};

int main() {
    Container<Base> cont;
    Derived1 d1;
    Derived2 d2;

    cont.add(&d1);
    cont.add(&d2);

    cont.printAll();  // Outputs: Derived1 class. Derived2 class.

    return 0;
}

```















----

## 备注/声明

1. 本文使用的模型数据由 [Vidar Rapp](https://se.linkedin.com/in/vidarrapp) 提供。
2. 本文框架基于https://github.com/ssloy/tinyrenderer。
