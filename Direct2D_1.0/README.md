## Direct2D_1.0
此目录为Direct2D_1.0版本的示例代码。</br>
[上一级](https://github.com/Ray1024/Direct2D)

## 一、基础示例

### 1.1 加载位图
(1)从文件加载位图(D2DCreateBitmapFromFile)</br>
(2)从资源加载位图(D2DCreateBitmapFromResource)</br>
详见[博客：Direct2d开发之从资源加载位图](http://www.cnblogs.com/Ray1024/p/6104368.html)。

### 1.2 将Direct2D位图保存为图片文件(D2DSaveBitmapToFile)
此示例演示了将窗口内容保存为图片文件。</br>
整个过程为：</br>
(1)将要绘制的内容绘制到CompatibleRenderTarget上；</br>
(2)从CompatibleRenderTarget上GetBitmap；</br>
(3)将获取的bitmap绘制到HwndRenderTarget上；</br>
(4)将获取的bitmap通过WIC组件保存为图片文件。</br>
注意：在此过程中，注意不同设备创建出来的资源之间的兼容，容易出错。

### 1.3 剪裁(D2DClip)
(1)轴对称矩形剪裁</br>
(2)几何图形蒙版剪裁</br>
![示例](https://github.com/Ray1024/PictureBed/blob/master/Direct2D/D2DClip.gif)

### 1.4 纹理混合
(1)对位图进行颜色混合(D2DBitmapBlend)</br>
对位图进行颜色混合，详见[博客：Direct2D开发之纹理混合](http://www.cnblogs.com/Ray1024/p/6189257.html)。</br>
![示例](https://github.com/Ray1024/PictureBed/blob/master/Direct2D/Direct2DBlend1.png)</br>

(2)将两个位图进行混合(D2DBitmapBlendWithBitmap)</br>
将两个位图进行混合，详见[博客：Direct2D开发之纹理混合](http://www.cnblogs.com/Ray1024/p/6189257.html)。</br>
![示例](https://github.com/Ray1024/PictureBed/blob/master/Direct2D/Direct2DBlend2.png)

### 1.5 自定义字体(DWriteCustomFont)
使用DirectWrite绘制自定义字体（即非系统字体）。</br>将自定义字体文件加载为资源文件也可以，但是在这里我是用的是另外一种方法。</br>方法是用字体文件和需要绘制的字符串创建路径几何图形（path geometry），然后直接绘制路径几何图形即可。

### 1.6 字符串文本去尾(DWriteStringTrimming)
使用DirectWrite绘制文本时，对字符串进行去尾操作，详见[博客：Direct2D开发之字符串去尾](http://www.cnblogs.com/Ray1024/p/5660490.html)。</br>
![示例](https://github.com/Ray1024/PictureBed/blob/master/Direct2D/DirectWriteStringTrimming.png)

### 1.7 特殊字符符号(DWriteCharacterSymbol)
DirectWrite使用"Segoe UI Symbol"字体来绘制一些字符符号。

### 1.8 测量文字大小
详见[博客：使用DirectWrite测量Direct2D文字大小](http://www.cnblogs.com/Ray1024/p/6027052.html)

### 1.9 使用平滑的曲线连接指定的几个点

### 1.10 [色度键过滤（绿幕抠图）](https://github.com/Ray1024/Direct2D/tree/master/Direct2D_1.0/D2DChromaKey)
绘制图片时，过滤掉指定颜色。（此方法直接操作像素，效率较低）</br>
![示例](https://github.com/Ray1024/PictureBed/blob/master/Direct2D/D2DChromaKey.png)

## 二、扩展示例

|项目名称|项目介绍|
| ----|----|
| D2DTMPL|Direct2D的一个模板框架程序|
| D2DBezierToWave|使用贝塞尔曲线绘制波形图。|
| D2DCollisionDetectionBetweenGeometrys|两个几何图形对象之间的位置关系判断，即碰撞检测，详见[博客：Direct2D处理几何图形之间的碰撞检测（下）](http://www.cnblogs.com/Ray1024/p/6030242.html)。|
| D2DGeometryCollisionDetectionWithPt|几何图形对象和点的位置关系判断，即碰撞检测，详见[博客：Direct2D处理几何图形之间的碰撞检测（上）](http://www.cnblogs.com/Ray1024/p/6028108.html)。|
| D2DGeometricTranslationOrder|在Direct2D绘图中，几何变换的顺序是有固定顺序的，正确的顺序应该是：缩放 -> 旋转 -> 平移。如果你随意改变几何变换的顺序，将会导致不可预见的错误结果。|
| D2DMesh|绘制网格，详见[博客：Direct2d开发之绘制网格](http://www.cnblogs.com/Ray1024/p/6103981.html)。|
| D2DLayeredWnd| Direct2D绘制分层窗口。|
