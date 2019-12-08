# RtspServer

## 一、项目介绍

- 使用C++实现的一个RTSP服务器

## 二、功能介绍

- 支持H264、AAC的音视频格式
- 支持传输H264格式的视频文件和AAC格式的音频文件
- 支持同时传输音视频
- 支持采集V4L2摄像头，编码成H264格式传输
- 支持采集ALSA音频设备，编码成AAC格式传输
- 支持单播(RTP_OVER_UDP, RTP_OVER_RTSP)，多播

## 三、开发环境

- 系统：Ubuntu 14.04
- 编译工具：gcc 4.8.4

## 四、总体框架

![frame](.\pic\frame.png)

## 五、使用方法

提供示例：传输H.264文件、传输AAC文件、同时传输H.264和AAC文件、采集摄像头数据编码传输、采集声卡数据编码传输

### 5.1 传输音视频文件

- 下载

  ```
  # git clone git@github.com:ImSjt/RtspServer.git
  ```

- 编译

  ```c
  # cd RtspServer/
  # make
  ```

  编译之后在`example/`目录下会生成`h264_rtsp_server`、`aac_rtsp_server`、`h264_aac_rtsp_server`

  **h264_rtsp_server**：传输H.264格式的视频文件

  **aac_rtsp_server**：传输AAC格式的音频文件

  **h264_aac_rtsp_server**：同时传输音视频

  另`v4l2_rtsp_server`和`alsa_rtsp_server`需要依赖别的库，默认不编译，稍后介绍

- 运行`h264_rtsp_server`

  进入`example`目录

  ```
  # cd example/
  ```

  运行`h264_rtsp_server`

  ```
  # ./h264_rtsp_server test.h264
  ```

  运行后会出现一行提示，其中url会随主机的ip改变

  ```
  Play the media using the URL "rtsp://192.168.31.115:8554/live"
  ```

  打开vlc，输入url，点击播放即可看到视频

  ![在这里插入图片描述](./pic/url.png)

- 效果
  ![在这里插入图片描述](./pic/show1.png)

- 运行`aac_rtsp_server`

  ```
  # ./aac_rtsp_server test.aac
  ```

- 运行`h264_aac_rtsp_server`

  ```
  # ./h264_aac_rtsp_server test.h264 test.aac
  ```


### 5.2 采集V4L2摄像头 

采集v4l2摄像头是`04_v4l2_rtsp_server.cpp`这个example，默认不会编译，这个示例相关的代码需要依赖x264库来编译，下面介绍使用步骤

- 编译安装x264

  如何编译x264，网上有很多教程了，这里就不详细讲解了

  将x264库放到Linux环境下，执行./configure，make，make install

  注意编译后的库和头文件要安装到`/usr/lib`和`/usr/include`下

- 修改Makefile

  将Makefile第一行

  ```
  V4L2_SUPPORT=n
  ```

  改为

  ```
  V4L2_SUPPORT=y
  ```

- 重新编译

  ```
  # make
  ```

  编译过后在`example/`目录下会出现`v4l2_rtsp_server`

- 运行`v4l2_rtsp_server`

  ```
  # ./v4l2_rtsp_server /dev/video0
  ```
  得到提示

  ```
  Play the media using the URL "rtsp://192.168.31.115:8554/live"
  ```

  在vlc输入url

- 效果

  ![在这里插入图片描述](./pic/show2.png)



### 5.3 采集ALSA音频设备

采集alsa音频设备是`05_alsa_rtsp_server.cpp`这个example，默认不会编译，这个示例相关的代码需要依赖alsa-lib和libfaac来编译，下面介绍使用步骤

- 编译安装alsa-lib和libfacc

  这里不详讲

  一般步骤，执行./configure，make，make install

  注意编译后的库和头文件要放到`/usr/lib`和`/usr/include`下

- 修改Makefile

  将Makefile第二行

  ```
  ALSA_SUPPORT=n
  ```

  修改为

  ```
  ALSA_SUPPORT=y
  ```

- 重新编译

  ```
  # make
  ```

  编译过后在`example/`目录下会出现`alsa_rtsp_server`

- 运行`alsa_rtsp_server`

  ```
  # ./alsa_rtsp_server hw:0,0
  ```

  得到提示

  ```
  Play the media using the URL "rtsp://192.168.31.115:8554/live"
  ```

  输入url即可得到声卡采集到的声音

### 5.4 RTP_OVER_RTSP

此项目默认时采用RTP_OVER_UDP，支持RTP_OVER_RTSP，如果需要测试，那么就需要设置vlc

`工具`>>`首选项`>>`输入/编解码器`>>`live555 流传输`>>`RTP over RTSP(TCP)`

![在这里插入图片描述](./pic/rtp2tcp.png)

然后随便运行一个example，在vlc输入url，此时就是使用RTP_OVER_RTSP

### 5.5 多播

如果想测试多播，就需要修改example的示例

打开任意一个示例，将其中

```
//session->startMulticast();
```

这段屏蔽打开，然后重新编译运行，即可切换到多播

## 六、技术点

- 服务器模型

  非阻塞IO，采用Reactor模型。使用线程池处理计算量比较大的任务（音视频文件处理，音视频数据采集与编码）

- IO多路复用

  支持select、poll、epoll

- 定时器

  通过Linux提供的定时器`timerfd_create `，将定时器文件描述符作为一个事件交给Reactor，定时器队列采用`multimap`管理超时时间

- 日志

  日志实现了前后端分离，前端负责格式化字符串然后发送给后端，后端启动一个线程，服务将数据写入磁盘中，使用了双缓冲技术

- 音视频数据采集与处理

  音视频的采集与处理使用的生产者与消费者模式，数据采集为生产者，数据处理为消费者。生产者维护着一个循环队列，会往线程池中添加任务填充缓存，消费者有一个定时器，间隔一定时间就会向生产者取数据，并将数据RTP打包再传输

  ![在这里插入图片描述](./pic/buffer.png)

- 内存管理

  内存管理分为前后端，前端使用类模板，灵活处理各种类型的对象的分配请求还有释放请求。后端主要是管理内存，提供内存的分配和释放。维护16个自由链表，每个链表维护着相同大小的内存块，分别从8、16、24到128。申请内存的时候，如果小于128字节则从相应的自由链表中获取内存块，如果大于128字节，则直接调用 malloc 进行分配。如果自由链表中没有内存块，那么就从缓冲区中申请一大块内存，然后切分成小块，插入到对应的自由链表中。释放内存的时候，如果小于128字节，那么就插入到对应的自由链表中，如果大于128字节，那么直接调用 free 释放内存

  ![](./pic/mem.png) 

## 七、联系方式

邮箱：1345648755@qq.com

博客：[程序员JT](https://blog.csdn.net/weixin_42462202) 
