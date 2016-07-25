#include "stdafx.h"
#include "flvoutboundstream.h"


FLVOutboundStream::FLVOutboundStream():FileOutboundStream()
{
    _prevTagSize = 0;
    _delta = -1;
}

FLVOutboundStream::~FLVOutboundStream() {
    if (_prevTagSize != 0) {
        if (!_file.WriteUI32(_prevTagSize)) {
            
        }
    }
	_file.Close();
}

bool FLVOutboundStream::Initialize( const string& fileName, bool append) 
{
    if (!FileOutboundStream::Initialize(fileName, append)) 
	{   
        return false;
    }

    if (!append) {
        //REFERENCE: video_file_format_spec_v10.pdf page 8/48
        const static string flv = "FLV";
        if (!_file.WriteString(flv))
            return false;

        if (!_file.WriteUI8(1))
            return false;

        if (!_file.WriteUI8(5))
            return false;

        if (!_file.WriteUI32(9))
            return false;
    }

    return true;
}

bool FLVOutboundStream::SaveData(uint8_t *pData, uint32_t length,uint32_t absoluteTime, bool isAudio) 
{
    if (_delta < 0)	
        _delta = absoluteTime;		

	//if(isAudio)
	//{
	//	char buf[128] = {0};
	//	sprintf(buf, "record audio ts:%d\n", absoluteTime);
	//	OutputDebugString(buf);

	//	static int counter = 0;
	//	if(counter%100 == 0)
	//	{
	//		char buf[128] = {0};
	//		sprintf(buf, "SaveData %d\n", GetTickCount());
	//		OutputDebugString(buf);
	//	}
	//	counter++;
	//}
	//else
	//{
	//	char buf[128] = {0};
	//	sprintf(buf, "record video ts:%d\n", absoluteTime);
	//	OutputDebugString(buf);
	//}

	/*FLVͷ�ļ�
	FLVͷ�ļ�:(9�ֽ�)
	1-3�� ǰ3���ֽ����ļ���ʽ��ʶ(FLV 0x46 0x4C 0x56).
	4-4�� ��4���ֽ��ǰ汾��0x01��
	5-5�� ��5���ֽڵ�ǰ5��bit�Ǳ����ı�����0.
	��5���ֽڵĵ�6��bit��Ƶ���ͱ�־(TypeFlagsAudio)
	��5���ֽڵĵ�7��bitҲ�Ǳ����ı�����0
	��5���ֽڵĵ�8��bit��Ƶ���ͱ�־(TypeFlagsVideo)
	6-9: ��6-9���ĸ��ֽڻ��Ǳ�����.������Ϊ 00000009 .
	�����ļ�ͷ�ĳ��ȣ�һ����9��3+1+1+4��

	[�༭]tag ������ʽ
	tag������Ϣ,�̶�����Ϊ15�ֽ�
	1-4: ǰһ��tag����(4�ֽ�),��һ��tag����0
	5-5: tag����(1�ֽ�);0x8��Ƶ��0x9��Ƶ��0x12�ű�����
	6-8: tag���ݴ�С(3�ֽ�)
	9-11: ʱ���(3�ֽ�)(����)(��1��tag��ʱ������Ϊ0,����ǽű�tag����0)
	12-12: ʱ�����չ(1�ֽ�)��ʱ������4�ֽ�(�Դ洢����ʱ���flvʱ����Ϣ),���ֽ���Ϊʱ��������λ.
	��flv�طŹ�����,����˳���ǰ���tag��ʱ���˳�򲥷�.�κμ��뵽�ļ���ʱ���������ݸ�ʽ����������.
	13-15: streamID(3�ֽ�) ����0*/


    if (!_file.WriteUI32(_prevTagSize)) 
        return false;
    

    if (!_file.WriteUI8(isAudio ? 8 : 9)) 	        
        return false;    

    if ( !_file.WriteUI24(length) ) 	        
        return false;    

    if (!_file.WriteSUI32((uint32_t) absoluteTime - (uint32_t) _delta)) {
        
        return false;
    }

    if (!_file.WriteUI24(0)) {
        
        return false;
    }

    if (!_file.WriteBuffer(pData,length)) {
        
        return false;
    }

    _prevTagSize = length + 11;

    return true;
}

















