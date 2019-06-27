# Wave_Audio
利用waveApi实现pcm录制和wav播放

wave_capture:通过waveIn实现录制.
wave_play：通过waveOut实现播放.



Windows上音频处理的API

在windos系统上，常用的音频处理技术主要包括：Wave系列API，DirectSound,Core Audio.

其中Core Audio只可以在Vista及以上系统中才能使用，主要用来取代Wave系列和DirectSound.

Core Aduio实现的功能比较强大，实现对麦克风采集，声卡输出的采集，控制声音的播放.

Wave系列API函数主要是用来实现对麦克风输入的采集(WaveIn),和控制声音的播放(WaveOut).



#### WaveIn系列API函数实现麦克风输入采集

- waveInOpen

  开启音频采集设备，成功后会返回设备句柄，后续的API都需要使用该句柄

  调用模块需要提供一个回调函数（waveInProc），以接收采集的音频数据

- waveInClose

  关闭音频采集模块

  成功后，由waveInOpen返回的设备句柄将不再有效 

- waveInPrepareHeader

  准备音频采集数据缓存的空间

- waveInUnprepareHeader

  清空音频采集的数据缓存

- waveInAddBuffer

  将准备好的音频数据缓存提供给音频采集设备

  在调用该API之前需要先调用waveInPrepareHeader

- waveInStart

  控制音频采集设备开始对音频数据的采集

- waveInStop

  控制音频采集设备停止对音频数据的采集

音频采集设备采集到音频数据后，会调用在waveInOpen中设置的回调函数。

其中参数包括一个消息类型，根据其消息类型就可以进行相应的操作。

如接收到WIM_DATA消息，则说明有新的音频数据被采集到，这样就可以根据需要来对这些音频数据进行处理。



#### 使用Core Audio实现对声卡输出的捕捉

涉及的接口有：

- IMMDeviceEnumerator
- IMMDevice
- IAudioClient
- IAudioCaptureClient

主要过程：

- 创建多媒体设备枚举器(IMMDeviceEnumerator)

- 通过多媒体设备枚举器获取声卡接口(IMMDevice)

- 通过声卡接口获取声卡客户端接口(IAudioClient)

- 通过声卡客户端接口(IAudioClient)可获取声卡输出的音频参数、初始化声卡、获取声卡输出缓冲区的大小、开启/停止对声卡输出的采集

- 通过声卡采集客户端接口(IAudioCaptureClient)可获取采集的声卡输出数据，并对内部缓冲区进行控制

  

### 4.常用的混音算法

混音算法就是将多路音频输入信号根据某种规则进行运算（多路音频信号相加后做限幅处理），得到一路混合后的音频，并以此作为输出的过程。

我目前还做过这一块，搜索了一下基本有如下几种混音算法：

- 将多路音频输入信号直接相加取和作为输出
- 将多路音频输入信号直接相加取和后，再除以混音通道数，防止溢出
- 将多路音频输入信号直接相加取和后，做Clip操作（将数据限定在最大值和最小值之间），如有溢出就设最大值
- 将多路音频输入信号直接相加取和后，做饱和处理，接近最大值时进行扭曲
- 将多路音频输入信号直接相加取和后，做归一化处理，全部乘个系数，使幅值归一化
- 将多路音频输入信号直接相加取和后，使用衰减因子限制幅值