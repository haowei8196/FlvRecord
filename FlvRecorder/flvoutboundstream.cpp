#include "stdafx.h"
#include "flvoutboundstream.h"
#include <assert.h>

FLVOutboundStream::FLVOutboundStream():FileOutboundStream()
, sound_format_(10)
, sound_rate_(3)	// 3 = 44 kHz
, sound_size_(1)	// 1 = 16-bit samples
, sound_type_(1)	// 0 = Mono sound; 1 = Stereo sound
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
void FLVOutboundStream::SetAudioParameter(int samplerate/*44100*/, int pcmbitsize/*16*/, int channel/*1*/)
{
	sound_samplerate_ = samplerate;
	switch (samplerate)
	{
	case 44100:
		sound_rate_ = 3;
		break;
	case 22050:
		sound_rate_ = 2;
		break;
	case 11025:
		sound_rate_ = 1;
		break;
	default:
		assert(false);
		break;
	};

	switch (pcmbitsize)
	{
	case 8:
		sound_size_ = 0;
		break;
	case 16:
		sound_size_ = 1;
		break;
	default:
		assert(false);
		break;
	}

	switch (channel)
	{
	case 1:
		sound_type_ = 0;
		break;
	case 2:
		sound_type_ = 1;
		break;
	default:
		assert(false);
		break;
	}
}
void FLVOutboundStream::SetVideoParameter(int width, int height, int videodatarate, int framerate)
{
	video_width_ = width;
	video_height_ = height;
	video_framerate_ = framerate;
	video_datarate_ = videodatarate;
}
uint8_t * put_byte(uint8_t *output, uint8_t nVal)
{
	output[0] = nVal;
	return output + 1;
}

uint8_t * put_be16(uint8_t *output, uint16_t nVal)
{
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output + 2;
}

uint8_t * put_be24(uint8_t *output, uint32_t nVal)
{
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;
	return output + 3;
}

uint8_t * put_be32(uint8_t *output, uint32_t nVal)
{
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return output + 4;
}

uint8_t *  put_be64(uint8_t *output, uint64_t nVal)
{
	output = put_be32(output, nVal >> 32);
	output = put_be32(output, nVal);
	return output;
}

uint8_t * put_amf_string(uint8_t *c, const char *str)
{
	uint16_t len = strlen(str);
	c = put_be16(c, len);
	memcpy(c, str, len);
	return c + len;
}

uint8_t * put_amf_double(uint8_t *c, double d)
{
	uint8_t *ci, *co;
	ci = (uint8_t *)&d;
	co = (unsigned char *)c;
	co[0] = ci[7];
	co[1] = ci[6];
	co[2] = ci[5];
	co[3] = ci[4];
	co[4] = ci[3];
	co[5] = ci[2];
	co[6] = ci[1];
	co[7] = ci[0];

	return c + 8;
}

#define AMF_STRING 0x02
#define AMF_STRING_LEN 0x0A
#define AMF_TYPE_ARRAY 0x08
#define AMF_OBJECT 0x03
#define AMF_OBJECT_END 0x09
bool FLVOutboundStream::SetMetaData()
{
	uint8_t body[512] = { 0 };
	uint8_t* p = (uint8_t*)body;
	p = put_byte(p, AMF_STRING);
	//p = put_be16(p, AMF_STRING_LEN);
	p = put_amf_string(p, "onMetaData");
	p = put_byte(p, AMF_TYPE_ARRAY);
	p = put_be32(p, 9);

	//p = put_be16(p, 5);
	p = put_amf_string(p, "width");
	p = put_byte(p, 0);
	p = put_amf_double(p, video_width_);

	//p = put_be16(p, 6);
	p = put_amf_string(p, "height");
	p = put_byte(p, 0);
	p = put_amf_double(p, video_height_);

	//p = put_be16(p, 9);
	p = put_amf_string(p, "videodatarate");
	p = put_byte(p, 0);
	p = put_amf_double(p, video_datarate_);

	p = put_amf_string(p, "framerate");
	p = put_byte(p, 0);
	p = put_amf_double(p, video_framerate_);

	p = put_amf_string(p, "videocodecid");
	p = put_byte(p, 0);
	p = put_amf_double(p, 7);

	//audiosamplerate
	p = put_amf_string(p, "audiosamplerate");
	p = put_byte(p, 0);
	p = put_amf_double(p, sound_samplerate_);
	//pcmbitsize
	p = put_amf_string(p, "audiosamplesize");
	p = put_byte(p, 0);
	p = put_amf_double(p, sound_size_);
	//channel
	p = put_amf_string(p, "stereo");
	p = put_byte(p, 0);
	p = put_amf_double(p, sound_type_);

	//sound_format_
	p = put_amf_string(p, "audiocodecid");
	p = put_byte(p, 0);
	p = put_amf_double(p, sound_format_);

	int len = p - body;
	if (!_file.WriteUI32(_prevTagSize))
	{
		return false;
	}

	if (!_file.WriteUI8(18))
	{
		return false;
	}

	if (!_file.WriteUI24(len))
	{
		return false;
	}

	if (!_file.WriteSUI32((uint32_t)(0)))
	{
		return false;
	}

	if (!_file.WriteUI24(0))
	{
		return false;
	}


	if (!_file.WriteBuffer((unsigned char*)body, len))
	{
		return false;
	}

	_prevTagSize = len + 11;
	_file.flush();
	return true;
}
bool FLVOutboundStream::SaveData(uint8_t *pData, uint32_t length,uint32_t absoluteTime, bool isAudio) 
{
	if (_delta < 0)
	{
		SetMetaData();
		_delta = absoluteTime;
	}
        		
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

















