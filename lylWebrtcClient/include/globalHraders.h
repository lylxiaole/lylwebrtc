
#ifndef globalHraders_H
#define globalHraders_H

//#define EX_API __declspec(dllexport)
#define EX_API extern "C" __declspec(dllexport)

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector> 
#include <objbase.h>

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "api/stats/rtcstatsreport.h"

typedef void(*_sdpCreateSuccess)(const char* sdp, const char* type);
typedef void(*_sdpCreateFailure)(const char* erro);
typedef void(*_setSdpSuccess)();
typedef void(*_setSdpFailure)(const char* error);

typedef void(*_onChannelStateChanged)(webrtc::DataChannelInterface::DataState state, char* channelLabel);
typedef void(*_onChannelMessage)(const char* buffer, int length, bool binary, char* channelLabel);
typedef void(*_onFrame)(uint8_t* imgdata, size_t datalength, int width, int height, const char* trackId);
typedef void(*_onSignalingState)(webrtc::PeerConnectionInterface::SignalingState state);



typedef void(*_onIceGatheringState)(webrtc::PeerConnectionInterface::IceGatheringState state);
typedef void(*_onRemoveStream)(char* streamLabel);
typedef void(*_onRemoveTrack)(char* streamLabel, char* trackId);
typedef void(*_onDataChannel)(char* data_channelId);
typedef void(*_onAddTrack)(char* streamLabel, char* trackId, char* trackType);
typedef void(*_onAddStream)(char* streamLabel);
typedef void(*_onIceConnectionState)(webrtc::PeerConnectionInterface::IceConnectionState state);
typedef void(*_onIceCandidate)(char* candidate, char* sdp_mid, int sdp_mline_index);
 
enum { kShortStringLength = 16, kMaxStringLength = 256, kMaxIceServerSize = 8 };


struct VedioDevice
{
	char deviceNameUTF8[kMaxStringLength];
	uint32_t deviceNameLength = 256;
	char deviceUniqueIdUTF8[kMaxStringLength];
	uint32_t deviceUniqueIdUTF8Length = 256;
	char productUniqueIdUTF8[kMaxStringLength];
	uint32_t productUniqueIdUTF8Length = 256;
};

struct AudioDeive
{
	char name[kMaxStringLength];
	char guid[kMaxStringLength];
};


void newGUID(char buf[64])
{
	memset(buf, 0, 64);
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buf, 64
			, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
		);
	}  
}

#endif