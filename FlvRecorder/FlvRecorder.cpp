// FlvRecorder.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "flvoutboundstream.h"

FLV_EXPORT IFlvRecorder*  GetRecorder()
{
	return new FlvRecorder();
}
FLV_EXPORT void           DelRecorder(IFlvRecorder* recorder)
{
	delete recorder;
	recorder = 0;
}
