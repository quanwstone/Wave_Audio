/*
 首先包含Windows.h，否则提示mmsystem.h编译失败.

 ffplay播放pcm文件：

 ffplay.exe -ar 8000 -ac 1 -f s16le -i c:\quanwei\BMP\1.pcm
 如果提示 Failed to open file filtergraph，则需要先输入如下命令
 set SDL_AUDIODRIVER=directsound
*/
#include<stdio.h>
#include<Windows.h>
#include<mmsystem.h>
#include<vector>
#pragma comment(lib ,"winmm.lib")

class CWaveCapture {
public:
	static void CALLBACK callbackProc(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	MMRESULT InitCapture();

	void Destory();

public:
	std::vector<LPWAVEHDR> m_vecHDR;

	HANDLE	m_hEvent;

	FILE *m_pFileWave;

	HWAVEIN m_pWaveIn;
};

void CWaveCapture::Destory()
{
	fclose(m_pFileWave);

	waveInClose(m_pWaveIn);
}
void CWaveCapture::callbackProc(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	printf("in callbackProc\n");

	CWaveCapture *pD = (CWaveCapture *)dwUser;

	if (uMsg == WIM_DATA)
	{
		pD->m_vecHDR.push_back((LPWAVEHDR)dw1);
		
		SetEvent(pD->m_hEvent);
	}
}
DWORD WINAPI Thread_Event(LPVOID param)
{
	CWaveCapture *pD = (CWaveCapture *)param;
	MMRESULT re;

	while (true)
	{
		WaitForSingleObject(pD->m_hEvent, INFINITE);

		LPWAVEHDR hd = *(pD->m_vecHDR).begin();
		
		pD->m_vecHDR.erase(pD->m_vecHDR.begin());
		
		waveInUnprepareHeader(pD->m_pWaveIn, hd, sizeof(WAVEHDR));

		fwrite(hd->lpData, 1, hd->dwBytesRecorded, pD->m_pFileWave);

		re = waveInPrepareHeader(pD->m_pWaveIn, hd, sizeof(WAVEHDR));
		if (re != MMSYSERR_NOERROR)
		{

		}
		re = waveInAddBuffer(pD->m_pWaveIn, hd, sizeof(WAVEHDR));
		if (re != MMSYSERR_NOERROR)
		{

		}
		ResetEvent(pD->m_hEvent);
	}
}
MMRESULT CWaveCapture::InitCapture()
{
	MMRESULT re = MMSYSERR_NOERROR;

	UINT uNum = waveInGetNumDevs();
	if (!uNum)
	{
		printf("waveInGetNumDevs Failed\n");
	}
	//
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	fopen_s(&m_pFileWave, "c:\\quanwei\\BMP\\1.pcm", "wb+");

	WAVEFORMATEX waveformat{ 0 };

	waveformat.nBlockAlign = 2;
	waveformat.nChannels = 1;
	waveformat.nSamplesPerSec = 8000;
	waveformat.nAvgBytesPerSec = 2 * 8000;
	waveformat.wBitsPerSample = 16;
	waveformat.cbSize = 0;
	waveformat.wFormatTag = WAVE_FORMAT_PCM;

	re = waveInOpen(&m_pWaveIn, WAVE_MAPPER, &waveformat, (DWORD)callbackProc, (DWORD)this, CALLBACK_FUNCTION);
	if (re != MMSYSERR_NOERROR)
	{
		printf("waveInOpen Failed\n");
	}
	CreateThread(NULL, 0, Thread_Event, this, 0, NULL);

	//Malloc HDR
	for (int i = 0; i < 2; i++)
	{
		LPWAVEHDR phdr = new WAVEHDR();

		phdr->dwBufferLength = waveformat.nAvgBytesPerSec;
		phdr->lpData = new char[phdr->dwBufferLength];
		memset(phdr->lpData, 0, phdr->dwBufferLength);
		phdr->dwBytesRecorded = 0;
		phdr->dwUser = 0;
		phdr->dwLoops = 0;
		phdr->lpNext = NULL;
		phdr->reserved = 0;

		re = waveInPrepareHeader(m_pWaveIn, phdr, sizeof(WAVEHDR));
		if (re != MMSYSERR_NOERROR)
		{

		}

		re = waveInAddBuffer(m_pWaveIn, phdr, sizeof(WAVEHDR));
		if (re != MMSYSERR_NOERROR)
		{

		}

	}

	re = waveInStart(m_pWaveIn);

	return re;
}

int main(int argc, char *argv[])
{
	
	CWaveCapture dlg;
	
	dlg.InitCapture();

	system("pause");

	dlg.Destory();

	return 0;
}