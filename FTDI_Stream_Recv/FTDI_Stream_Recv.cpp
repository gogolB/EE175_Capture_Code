// FTDI_Stream_Recv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include "ftd2xx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int main()
{
	printf("Test program to see stream speed for FTDI USB Module. \nVersion: 1.0\nUses magic char 0xAA\n");

	FT_HANDLE ft_handle;
	FT_STATUS ft_status;

	UCHAR mask = 0xFF;
	UCHAR mode;
	UCHAR LatencyTimer = 2;

#define WIDTH 640
#define HEIGHT 480
	// OpenCv stuff
	Mat img(HEIGHT,WIDTH,CV_8UC1);

	namedWindow("Captured Data", WINDOW_AUTOSIZE);
	ft_status = FT_Open(2, &ft_handle);
	if (ft_status != FT_OK)
	{
		// Failed to open device
		printf("Failed to open device\n");
		return -1;
	}

	// Put the device in reset mode
	mode = 0x00;
	ft_status = FT_SetBitMode(ft_handle, mask, mode);
	Sleep(10);
	// Put the device in Sync FIFO mode
	mode = 0x40;
	ft_status = FT_SetBitMode(ft_handle, mask, mode);

	if (ft_status == FT_OK)
	{
		ft_status = FT_SetLatencyTimer(ft_handle, LatencyTimer);
		ft_status = FT_SetFlowControl(ft_handle, FT_FLOW_RTS_CTS, 0, 0);
		ft_status = FT_SetUSBParameters(ft_handle, 0x10000, 0x10000);

		DWORD EventDWord;
		DWORD RxBytes;
		DWORD TxBytes;
		DWORD BytesRecv;
		DWORD BytesWritten;
		
#define ONE_IMAGE HEIGHT*WIDTH

		char rxBuffer[ONE_IMAGE];
		LARGE_INTEGER iPreTime, IPostTime, IFrequency;

		char txBuffer[1];
		txBuffer[0] = 0xAA;

		// Access Data.
		printf("Waiting to send data...\n");
		getchar();
		printf("Entering Gathering loop\n");
		ft_status = FT_GetStatus(ft_handle, &RxBytes, &TxBytes, &EventDWord);
		if ((ft_status == FT_OK) && (TxBytes == 0))
		{
			ft_status = FT_Write(ft_handle, txBuffer, sizeof(txBuffer), &BytesWritten);
			if(ft_status != FT_OK)
			{
				printf("Write failed...\nTest Aborting...\n");
				return -2;
			}
		}

		while (true)
		{
			QueryPerformanceFrequency(&IFrequency);
			QueryPerformanceCounter(&iPreTime);
			DWORD dwSum = 0;
			DWORD dwCurrentOffset = 0;

			for (int i = 0; i < 30; i++)
			{
				while (dwCurrentOffset < ONE_IMAGE)
				{
					ft_status = FT_Read(ft_handle, rxBuffer + dwCurrentOffset, ONE_IMAGE - dwCurrentOffset, &BytesRecv);

					if (ft_status == FT_OK)
					{
						// Read was okay
						//printf("Read one image\n");
						dwSum += BytesRecv;
						dwCurrentOffset += BytesRecv;
						//printf("Read %d bytes current total at %d\n", BytesRecv, dwCurrentOffset);
					}
				}
				//printf("Frame Count = %d\n", i);
				dwCurrentOffset = 0;

				// Display the image
				std::memcpy(img.data, rxBuffer, ONE_IMAGE);
				imshow("Captured Data", img);
				waitKey(1);
			}

			QueryPerformanceCounter(&IPostTime);
			float IPassTick = IPostTime.QuadPart - iPreTime.QuadPart;
			double PCFrq = IFrequency.QuadPart / 1000.0;
			float IPassTime = IPassTick / PCFrq;
			IPassTime /= 1000.0;
			float bps = ((float)dwSum) / IPassTime;
			float mbps = bps / 1e6;
			float fps = 30.0 / IPassTime;
			printf("Recv'd %d bytes in %f seconds\n\t(%f MBps)\n", dwSum, IPassTime, mbps);
			printf("FPS: %f\n", fps);
		}
	}
	else
	{
		// Failed to set bit mode
		printf("Failed to set bit mode\n");
		return -1;
	}
	
	FT_Close(ft_handle);

    return 0;
}

