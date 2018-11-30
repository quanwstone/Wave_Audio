/*
利用waveOut实现声音的输出。 

*/
#include<stdio.h>
#include<Windows.h>
#include<mmsystem.h>
#include<vector>

#pragma comment(lib ,"winmm.lib")

struct STR_CONTEXT {
	LPWAVEHDR hd;
	bool iUser;
};
class CPlay {
public:
	MMRESULT InitPlay(unsigned long ulHz, unsigned short iCh,unsigned short usBit);

	MMRESULT AllocQueue(DWORD dLen);

	LPWAVEHDR GetFreeHdr();

	MMRESULT PutWaveOut(unsigned char *ch,unsigned int iLen);

	void Destory();
	static void (CALLBACK callbackProc)(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
public:

	HWAVEOUT	m_phOut;
	std::vector<STR_CONTEXT *>m_vectQueue;
	CRITICAL_SECTION	m_csLock;
};

void CPlay::callbackProc(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	CPlay *dlg = (CPlay*)dwUser;

	if (uMsg == WOM_DONE)
	{
		printf("in callbackProc\n");

		//回调函数调用该方法是否会存在死锁的情况。 
		waveOutUnprepareHeader(dlg->m_phOut, (LPWAVEHDR)dw1, sizeof(WAVEHDR));

		EnterCriticalSection(&dlg->m_csLock);
		for (std::vector<STR_CONTEXT *>::iterator iter = dlg->m_vectQueue.begin();
			iter != dlg->m_vectQueue.end(); iter++)
		{
			if ((*iter)->hd == (LPWAVEHDR)dw1)
			{
				(*iter)->iUser = false;
			}
		}
		LeaveCriticalSection(&dlg->m_csLock);
	}
}
void CPlay::Destory()
{
	DeleteCriticalSection(&m_csLock);

	waveOutClose(m_phOut);
}
LPWAVEHDR CPlay::GetFreeHdr()
{
	for (std::vector<STR_CONTEXT *>::iterator iter = m_vectQueue.begin(); iter != m_vectQueue.end(); iter++)
	{
		if ((*iter)->iUser == false)
		{
			(*iter)->iUser = true;
			return (*iter)->hd;
		}
	}
	return nullptr;
}
MMRESULT CPlay::PutWaveOut(unsigned char *ch, unsigned int iLen)
{
	MMRESULT re = MMSYSERR_NOERROR;
	
	LPWAVEHDR lp = nullptr;
	do
	{
		EnterCriticalSection(&m_csLock);
		lp = GetFreeHdr();
		LeaveCriticalSection(&m_csLock);

		if (!lp)
		{
			Sleep(1000);
		}
		
	} while (!lp);
	
	lp->dwBufferLength = iLen;
	lp->dwFlags = 0;
	memcpy(lp->lpData, ch, iLen);


	re = waveOutPrepareHeader(m_phOut, lp, sizeof(WAVEHDR));
	if (re != MMSYSERR_NOERROR)
	{
		printf("Play Failed\n");
	}

	re = waveOutWrite(m_phOut, lp, sizeof(WAVEHDR));//WAVERR_UNPREPARED 
	if (re != MMSYSERR_NOERROR)
	{
		printf("PutWaveOut Failed\n");
	}

	return re;
}
MMRESULT CPlay::AllocQueue(DWORD dLen)
{
	MMRESULT re = MMSYSERR_NOERROR;

	//申请两个的原因是waveOutWrite内部会缓存一个HDR.   
	for (int i = 0; i < 2; i++)
	{
		STR_CONTEXT *sCon = new STR_CONTEXT;

		sCon->hd = new WAVEHDR();
		sCon->hd->lpData = new char[dLen];
		sCon->hd->dwFlags = 0;
		sCon->iUser = false;

		memset(sCon->hd->lpData, 0, dLen);

		m_vectQueue.push_back(sCon);
	}
	return re;
}
MMRESULT CPlay::InitPlay(unsigned long ulHz, unsigned short iCh, unsigned short usBit)
{
	MMRESULT re = MMSYSERR_NOERROR;

	UINT inum = waveOutGetNumDevs();
	if (!inum)
	{
		printf("waveOutGetNumDevs Failed\n");
	}
	
	WAVEFORMATEX format{0};
	format.cbSize = 0;
	format.nBlockAlign = iCh * usBit / 8;		//字节对齐
	format.wBitsPerSample = usBit;				//每个sample占用的bit数
	format.nChannels = iCh;						//声道数
	format.nSamplesPerSec = ulHz;				//每秒的sample个数
	format.nAvgBytesPerSec = iCh * format.nBlockAlign * format.nSamplesPerSec;//每秒占用的字节数
	format.wFormatTag = WAVE_FORMAT_PCM;

	re = waveOutOpen(&m_phOut, WAVE_MAPPER, &format, (DWORD)callbackProc, DWORD(this), CALLBACK_FUNCTION);
	if (re != MMSYSERR_NOERROR)
	{
		printf("waveOutOpen Failed\n");
	}
	
	InitializeCriticalSection(&m_csLock);

	
	re = AllocQueue(format.nAvgBytesPerSec);
	if (re != MMSYSERR_NOERROR)
	{
		printf("AllocQueue Failed\n");
	}

	return re;
}
int main(int argc, char *argv[])
{
	MMRESULT re = MMSYSERR_NOERROR;
	
	CPlay dlg;
	
	unsigned short uiChannel = 1;
	unsigned long  ulHz = 8000;
	unsigned short uiBit = 2;

	FILE *pf = nullptr;

	fopen_s(&pf, "C:\\quanwei\\Media\\trunk\\Capture-Doc\\AudioCapture\\机器B.wav", "rb");

	fseek(pf, 22, SEEK_SET);
	fread(&uiChannel, 1, 2, pf);

	fseek(pf, 24, SEEK_SET);
	fread(&ulHz, 1, 4, pf);

	fseek(pf, 34, SEEK_SET);
	fread(&uiBit, 1, 2, pf);


	fseek(pf, 44, SEEK_SET);

	re = dlg.InitPlay(ulHz, uiChannel,uiBit);

	int iLen = ulHz * uiBit / 8;

	unsigned char *buf = new unsigned char[iLen];
	memset(buf, 0, iLen);

	while (fread(buf, 1, iLen, pf))
	{
		//printf("Read = %d\n",i);

		re = dlg.PutWaveOut(buf, iLen);

		if (re != MMSYSERR_NOERROR)
		{
			printf("PutWaveOut Failed\n");
		}
	}

	fclose(pf);
	
	dlg.Destory();

	return 0;
}