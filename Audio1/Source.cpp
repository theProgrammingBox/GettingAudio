//make a program that reads the mic and turns the audio data into a real time spectogram

#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <Windows.h>

using std::wstring;
using std::cout;
using std::endl;
using std::wcout;
using std::cin;

// convert a range of numbers into ascii to represent the range of values in terminal
wstring GetVisual(wstring characters, float value, float min, float max)
{
	int range = max - min;
	int index = (value - min) / range * (characters.length() - 1);
	return characters.substr(index, 1);
}

int main()
{
	// get the default audio device
	wstring defaultDeviceID = L"";
	{
		WAVEOUTCAPS woc;
		for (UINT i = 0; i < waveOutGetNumDevs(); i++)
		{
			waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS));
			if (woc.dwFormats & WAVE_FORMAT_1M08)
			{
				defaultDeviceID = woc.szPname;
				break;
			}
		}
	}

	// open the default audio device
	HWAVEIN hWaveIn;
	WAVEFORMATEX waveFormat;
	{
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 1;
		waveFormat.nSamplesPerSec = 44100;
		waveFormat.wBitsPerSample = sizeof(short) * 8;
		waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = 0;

		waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL);
	}

	float seconds = 1.0f / 60.0f;	//60 fps
	
	// create a recording buffer
	WAVEHDR waveHeader;
	{
		waveHeader.dwBufferLength = waveFormat.nAvgBytesPerSec * seconds;
		waveHeader.lpData = new char[waveHeader.dwBufferLength];
		waveHeader.dwBytesRecorded = 0;
		waveHeader.dwUser = 0;
		waveHeader.dwFlags = 0;
		waveHeader.dwLoops = 0;
		waveHeader.lpNext = 0;
		waveHeader.reserved = 0;

		waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));
	}

	int i = 0;
	while (true)
	{
		// start recording
		waveInStart(hWaveIn);

		// wait for recording to finish
		while (waveHeader.dwBytesRecorded < waveHeader.dwBufferLength) {}

		// stop recording
		waveInStop(hWaveIn);

		// get the recorded data
		short* samples = (short*)waveHeader.lpData;
		int sampleCount = waveHeader.dwBytesRecorded / sizeof(short);
		
		float minValue = INFINITY;
		float maxValue = -INFINITY;
		for (int i = 0; i < sampleCount; i++)
		{
			if (samples[i] < minValue) minValue = samples[i];
			if (samples[i] > maxValue) maxValue = samples[i];
		}

		//cout << "min: " << minValue << " max: " << maxValue << endl;

		for (int i = 0; i < sampleCount; i++)
		{
			wcout << GetVisual(L".:-=+*#%@", samples[i], minValue, maxValue);
		}
		
		// reset the recording buffer
		waveHeader.dwBytesRecorded = 0;
		waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));
	}
}