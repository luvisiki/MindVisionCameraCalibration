//---------------------------------------------------------------------------
//BIG5 TRANS ALLOWED
#include <vcl.h>
#pragma hdrstop

#include "mainform.h"
#include "stdio.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "..\VC++\Include\CameraApi.h"
#include <process.h>
                             
TForm1 *Form1;
//������  �������ͬʱ���ʣ�ͨ����ͬ�ľ�����֡�
int m_hCamera = 0;
//�����ͼ�񻺳���
unsigned char* m_pRgbBuffer = NULL;
//�����ǰ��ʾ��ͼ����֡��
int  m_iDispNum = 0;
//�ϴ�ͳ��ʱ�������ʾ����֡��
int  m_iLastDisNum  = 0;
//ץͼ�̵߳ľ��
HANDLE  m_hDispThread = NULL;
//ץͼ�߳��˳���־
int m_bExit = 0;
//��һ֡��ͷ��Ϣ
tSdkFrameHead m_sFrInfo;
//ץͼ�߳�ID
UINT m_threadID;

UINT WINAPI uCaptureThread(LPVOID lpvParam)
{
	TForm1* Form1 = (TForm1*)lpvParam;
	BYTE      *pbyBuffer;
	CameraSdkStatus status;


	tSdkFrameHead sFrameInfo;

	while (!m_bExit)
        {
		if(CameraGetImageBuffer(m_hCamera,&sFrameInfo,&pbyBuffer,1000) == CAMERA_STATUS_SUCCESS)
		{

			status = CameraImageProcess(m_hCamera, pbyBuffer, m_pRgbBuffer,&sFrameInfo);

			if (m_sFrInfo.iWidth != sFrameInfo.iWidth || m_sFrInfo.iHeight != sFrameInfo.iHeight)//�л��ֱ��ʴ�Сʱ���������ಿ�֡�
			{
				m_sFrInfo.iWidth = sFrameInfo.iWidth;
				m_sFrInfo.iHeight = sFrameInfo.iHeight;
                                Form1->Panel1->Invalidate();
			}

                        CameraDisplayRGB24(m_hCamera, m_pRgbBuffer, &sFrameInfo);
                        m_iDispNum++;


                        CameraReleaseImageBuffer(m_hCamera,pbyBuffer);

			memcpy(&m_sFrInfo,&sFrameInfo,sizeof(tSdkFrameHead));
		}

		
    }

	_endthreadex(0);

    return 0;
}


//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{

        if (m_hCamera > 0)
        {
             Label1->SetTextBuf("123");
        }
}
//----------���º���ͣ-----------------------------------------------------------------//----------���º����Ԥ������Ĭ�ϴ�DEMO���Զ�Ԥ����-----------------------------------------------------------------
//----------���º󱣴�ͼƬ���ļ���-----------------------------------------------------------------
//----------���º󵯳�������öԻ���-----------------------------------------------------------------


void __fastcall TForm1::FormActivate(TObject *Sender)
{
        int n;
        CameraSdkStatus status;
        tSdkCameraCapbility sCameraCap;

        tSdkCameraDevInfo tEnumInfos[4];
        n = 4;
        //ö���豸�����û�ҵ���������ӡ������Ϣ���˳�
        CameraEnumerateDevice(tEnumInfos,&n);
        if (n == 0)
        {
           ShowMessage("No camera!");
           return;
        }

        status =  CameraInit(&tEnumInfos[0], -1,-1 ,&m_hCamera);

        if (CAMERA_STATUS_SUCCESS != status)
        {

           char msg[64];
           sprintf(msg,"Camera init failed ,error code is %d",status);
           ShowMessage(msg);
           return;
        }

         
        CameraGetCapability(m_hCamera, &sCameraCap);

        //����һ��RGB24��buffer
        m_pRgbBuffer = (BYTE *)CameraAlignMalloc(sCameraCap.sResolutionRange.iWidthMax*
        sCameraCap.sResolutionRange.iHeightMax*3,16);
        //���ϣ�����RGB32�ĸ�ʽ������Ҫ����sCameraCap.sResolutionRange.iWidthMax*
        //sCameraCap.sResolutionRange.iHeightMax*4��С��buffer

         CameraDisplayInit(m_hCamera,Panel1->Handle);

	 CameraSetDisplaySize(m_hCamera,Panel1->Width,Panel1->Height);

         CameraCreateSettingPage(m_hCamera,Form1->Handle,tEnumInfos[0].acFriendlyName,NULL,0,0);

         m_bExit = 0;
         m_hDispThread = (HANDLE)_beginthreadex(NULL, 0, &uCaptureThread, (VOID*)Form1, 0, (unsigned int*) &m_threadID);

         //�������ʼ�ɼ�ͼ��
         CameraPlay(m_hCamera);

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button_settingsClick(TObject *Sender)
{
        //�����ڲ����ɵ�������ÿ�������á������Ҫ���ú�������ĳ����������ο�API�ֲᣬֱ�ӵ��ü��ɡ�
        if (m_hCamera > 0)
        {
             CameraShowSettingPage(m_hCamera,1);
        }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
        if(m_hCamera > 0)
        {
            m_bExit = 1;  //�òɼ��߳��˳�
            WaitForSingleObject(m_hDispThread, INFINITE);
            
            CameraUnInit(m_hCamera);
            
            m_hCamera = 0;
        }

        if (m_pRgbBuffer)
        {
            CameraAlignFree(m_pRgbBuffer);
            m_pRgbBuffer = NULL;
        }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button_pauseClick(TObject *Sender)
{
        if (m_hCamera > 0)
        {
            CameraPause(m_hCamera);
        }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button_StartClick(TObject *Sender)
{
        if (m_hCamera > 0)
        {
            CameraPlay(m_hCamera);
        }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
        char msg[128];
        tSdkFrameStatistic sFrameCount;
        static int iLastCaptureFrames = 0;
        CameraGetFrameStatistic(m_hCamera, &sFrameCount);

        sprintf(msg,"ͼ��ߴ磺%d X %d|��ʾ��֡��:%d|�ɼ�֡��:%d|��ʾ֡��:%d",
        m_sFrInfo.iWidth,m_sFrInfo.iHeight,sFrameCount.iTotal ,sFrameCount.iTotal - iLastCaptureFrames,m_iDispNum - m_iLastDisNum);
        m_iLastDisNum = m_iDispNum;
        iLastCaptureFrames =  sFrameCount.iTotal;

        Label1->SetTextBuf(msg);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button_snapshotClick(TObject *Sender)
{
        tSdkFrameHead FrameInfo;
	BYTE *pRawBuffer;
	BYTE *pRgbBuffer;
        char msg[64];

        tSdkImageResolution sImageSize;
        CameraSdkStatus status;

        memset(&sImageSize,0,sizeof(tSdkImageResolution));
	sImageSize.iIndex = 0xff;  //���ó�0xff��ʾ��Ԥ��һ���ķֱ���ץ��
        //����ץ�ĵķֱ���
        CameraSetResolutionForSnap(m_hCamera,&sImageSize);

		// !!!!!!ע�⣺CameraSnapToBuffer ���л��ֱ������գ��ٶȽ�������ʵʱ��������CameraGetImageBuffer����ȡͼ���߻ص�������
        if((status = CameraSnapToBuffer(m_hCamera,&FrameInfo,&pRawBuffer,1000)) != CAMERA_STATUS_SUCCESS)
	{

		ShowMessage("Snapshot failed,is camera in pause mode?");
		return;
	}
	else
	{

		//�ɹ�ץ�ĺ󣬱��浽�ļ�


		//����һ��buffer����������õ�ԭʼ����ת��ΪRGB���ݣ���ͬʱ���ͼ����Ч��
		pRgbBuffer = (BYTE *)CameraAlignMalloc(FrameInfo.iWidth*FrameInfo.iHeight*3,16);
		//Process the raw data,and get the return image in RGB format
		CameraImageProcess(m_hCamera,pRawBuffer,pRgbBuffer,&FrameInfo);

		 //Release the buffer which get from CameraSnapToBuffer or CameraGetImageBuffer
		CameraReleaseImageBuffer(m_hCamera,pRawBuffer);

		//CameraSaveImage ����ͼ�����������ʾ��α���BMPͼ�������Ҫ�����������ʽ�ģ������JPG,PNG,RAW�ȣ�
		//��ο����ǵ�Snapshot���̡����߲���SDK�ֲ����й�CameraSaveImage�ӿڵ���ϸ˵��
		if((status = CameraSaveImage(m_hCamera,"C:\\test.bmp",pRgbBuffer,&FrameInfo,FILE_BMP,100)) != CAMERA_STATUS_SUCCESS)
		{
			ShowMessage("Failed to save image");
		}
		else
		{
			sprintf(msg,"Image save in:[%s]","C\\test.bmp");
			ShowMessage(msg);
		}

		CameraAlignFree(pRgbBuffer);
		
	}
}
//---------------------------------------------------------------------------
