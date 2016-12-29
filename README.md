# Direct2DTests
这个仓库包含了我学习Direct2D过程中写的示例代码，会不定时更新。</br>
大部分示例代码在[我的博客](http://www.cnblogs.com/Ray1024/)中都有详细解释。

|项目名称|项目介绍|
| ----|----|
| D2DBezierToWave|此项目展示了在Direct2D中如何使用贝塞尔曲线绘制波形图。|
| D2DBitmapBlend|此项目展示了如何对位图进行颜色混合，详见[博客：Direct2D开发之纹理混合](http://www.cnblogs.com/Ray1024/p/6189257.html)。|
| D2DBitmapBlendWithBitmap|此项目展示了将两个位图进行混合并显示的效果，详见[博客：Direct2D开发之纹理混合](http://www.cnblogs.com/Ray1024/p/6189257.html)。|
| D2DCollisionDetectionBetweenGeometrys|此项目展示了两个几何图形对象之间的位置关系判断，即碰撞检测，详见[博客：Direct2D处理几何图形之间的碰撞检测（下）](http://www.cnblogs.com/Ray1024/p/6030242.html)。|
| D2DCreateBitmapFromResource|此项目展示了如何从资源文件加载位图，详见[博客：Direct2d开发之从资源加载位图](http://www.cnblogs.com/Ray1024/p/6030242.html)。|
| D2DFPS|此项目是关于帧数的一个测试程序。|
| D2DFrameApp|此项目是我整理的一个Direct2D的简单程序框架，可以自动计算帧数。|
| D2DGeometricTranslationOrder|此项目展示了在Direct2D绘图中，几何变换的顺序是有固定顺序的，正确的顺序应该是：缩放 -> 旋转 -> 平移。如果你随意改变几何变换的顺序，将会导致不可预见的错误结果。|
| D2DGeometryCollisionDetectionWithPt|此项目展示了几何图形对象和点的位置关系判断，即碰撞检测，详见[博客：Direct2D处理几何图形之间的碰撞检测（上）](http://www.cnblogs.com/Ray1024/p/6028108.html)。|
| D2DMesh|此项目展示了在Direct2D中如何绘制网格，详见[博客：Direct2d开发之绘制网格](http://www.cnblogs.com/Ray1024/p/6103981.html)。|
| D2DTMPL|这个项目是本仓库中大部分示例程序都使用的一个空框架程序。|
| DWriteCustomFont|此项目展示了在DirectWrite中绘制自定义字体（即非系统字体）。</br>将自定义字体文件加载为资源文件也可以，但是在这里我是用的是另外一种方法。</br>方法是用字体文件和需要绘制的字符串创建路径几何图形（path geometry），然后直接绘制路径几何图形即可。|
| DWriteStringTrimming|此项目展示了使用DirectWrite绘制文本时，对字符串进行去尾操作，详见[博客：Direct2D开发之字符串去尾](http://www.cnblogs.com/Ray1024/p/5660490.html)。|
