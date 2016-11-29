#include <windows.h>
#include "D2DTimer.h"

//
// D2DTimer Implement
//

D2DTimer::D2DTimer()
	: m_secondsPerCount(0.0)
	, m_deltaTime(-1.0)
	, m_baseTime(0)
	, m_pausedTime(0)
	, m_prevTime(0)
	, m_currTime(0)
	, m_stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}

float D2DTimer::DeltaTime() const
{ 
	return (float)m_deltaTime; 
}

// 返回自调用Reset()方法之后的总时间，不包含暂停时间
float D2DTimer::TotalTime()const
{
	// 如果处在暂停状态，则无需包含自暂停开始之后的时间。
	// 此外,如果我们之前已经有过暂停,则m_stopTime - m_baseTime会包含暂停时间, 我们不想包含这个暂停时间，
	// 因此还要减去暂停时间：  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  m_baseTime       m_stopTime        startTime     m_stopTime    m_currTime

	if( m_stopped )
	{
		return (float)(((m_stopTime - m_pausedTime)-m_baseTime)*m_secondsPerCount);
	}

	// m_currTime - m_baseTime包含暂停时间,而我们不想包含暂停时间，
	// 因此我们从m_currTime需要减去m_pausedTime：
	//
	//  (m_currTime - m_pausedTime) - m_baseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  m_baseTime       m_stopTime        startTime     m_currTime

	else
	{
		return (float)(((m_currTime-m_pausedTime)-m_baseTime)*m_secondsPerCount);
	}
}

void D2DTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_stopped  = false;
}

void D2DTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// 累加暂停与开始之间流逝的时间
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  m_baseTime       m_stopTime        startTime     

	// 如果仍处在暂停状态
	if( m_stopped )
	{
		// 则累加暂停时间
		m_pausedTime += (startTime - m_stopTime); 
		// 因为我们重新开始计时，因此m_prevTime的值就不正确了，
		// 要将它重置为当前时间 
		m_prevTime = startTime;
		// 取消暂停状态
		m_stopTime = 0;      
		m_stopped  = false;
	}
}

void D2DTimer::Stop()
{
	// 如果正处在暂停状态，则略过下面的操作
	if( !m_stopped )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// 记录暂停的时间，并设置表示暂停状态的标志
		m_stopTime = currTime;
		m_stopped  = true;
	}
}

void D2DTimer::Tick()
{
	if( m_stopped )
	{
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	// 当前帧和上一帧之间的时间差
	m_deltaTime = (m_currTime - m_prevTime)*m_secondsPerCount;

	// 为计算下一帧做准备
	m_prevTime = m_currTime;

	// 确保不为负值。DXSDK中的CDXUTTimer提到：如果处理器进入了节电模式
	// 或切换到另一个处理器，m_deltaTime会变为负值。
	if(m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}
}