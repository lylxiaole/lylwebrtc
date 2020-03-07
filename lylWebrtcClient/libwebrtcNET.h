#ifndef libwebrtcNET_H
#define libwebrtcNET_H  

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/test/fakeaudiocapturemodule.h"
#include "p2p/base/fakeportallocator.h"

#include "globalHraders.h"  
#include "lylPeerConnection.h" 
#include "lylVideoRender.h"

std::vector< rtc::scoped_refptr<lylwebrtc::lylPeerConnection>> peer_connections;
rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceModule_ = nullptr;
webrtc::VideoCaptureModule::DeviceInfo* videoDeviceModule_ = nullptr;
 
std::unique_ptr<rtc::Thread> worker_thread;
std::unique_ptr<rtc::Thread> signaling_thread;
std::unique_ptr<rtc::Thread> network_thread;
 
rtc::scoped_refptr <lylwebrtc::lylPeerConnection> findPeerConnection(void * paddr);
//创建一个链接工厂类
//*/
EX_API void Initialize();
EX_API void RealseAll();


EX_API void CloseConnection(void* peeraddr);
EX_API void* CreatePeerConnection();
EX_API void RegisterIceServer(const char uri[kMaxStringLength], const char username[kMaxStringLength], const char password[kMaxStringLength], void* peeraddr);
EX_API void SetPeerConnectionConfig(int icetype, void* peeraddr);
EX_API void SetTrackDevice(bool isVedio, int vedioIndex, bool isAudio, int recordAudioIndex, bool isDesktop, void* peeraddr);

//获取视频输入设备的数量
EX_API int32_t GetVideoDeviceNumber();
//根据Index获取视频输出设备信息
EX_API void GetVideoDevice(int32_t index, VedioDevice& refdeviceInfo);
//获取音频输出设备的数量
EX_API int16_t GetPlayoutAudioDeviceNumber();
//根据Index获取音频输出设备信息
EX_API void GetPlayoutAudioDevice(int32_t index, AudioDeive& refAudioDevice);
//获取音频输入设备的数量
EX_API int16_t GetRecordingAudioDeviceNumber();
EX_API void SetPlayoutAudioDevice(uint32_t playoutAudioIndex);
//根据Index获取音频输入设备信息
EX_API void GetRecordingAudioDevice(int32_t index, AudioDeive& refAudioDevice);
//*************信令相关*********************************************************
EX_API void CreateOffer(_sdpCreateSuccess onsuccess, _sdpCreateFailure onfailure, void* peeraddr);
EX_API void AddCandidate(char* candiate, char* mid, int midx, void* peeraddr);
EX_API void SetRemoteDescription(const char* offer_sdp, const char* type, _sdpCreateSuccess onCreateSdpSuccess, _sdpCreateFailure onCreateSdpFailure, void* peeraddr);
EX_API void DealCacheIceCandidate(void* peeraddr);
//*************管道****************************************************************
EX_API char* CreateDataChannel(char* channelLabel, void* peeraddr);
EX_API void SetDataChannelEvent(_onChannelStateChanged onStateChangeHandle, _onChannelMessage onMessageHandle, char* channelId, void* peeraddr);
EX_API void SendChannelData(char* channelId, const char* data, void* peeraddr);
EX_API void RemoveDataChannel(char* channelId, void* peeraddr);

EX_API void  RemoveStream(char* streamId, void* peeraddr);
//********事件************************************************************** 
EX_API void SetListenerSignalingState(_onSignalingState handle, void* peeraddr);
EX_API void SetListenerIceGatheringState(_onIceGatheringState handle, void* peeraddr);
EX_API void SetListenerRemoveStream(_onRemoveStream handle, void* peeraddr);
EX_API void SetListenerRemoveTrack(_onRemoveTrack handle, void* peeraddr);
EX_API void SetListenerOnNewDataChannel(_onDataChannel handle, void* peeraddr);
EX_API void SetListenerAddTrack(_onAddTrack handle, void* peeraddr);
EX_API void SetListenerAddStream(_onAddStream handle, void* peeraddr);
EX_API void SetListenerIceConnectionState(_onIceConnectionState handle, void* peeraddr);
EX_API void SetListenerIceCandidate(_onIceCandidate onIceCandidate, void* peeraddr);
EX_API void SetListenonLocalFrame(_onFrame onFrameMethod, void* peeraddr);
EX_API void SetListenonRemoteFrame(_onFrame onFrameMethod, char* trackId, void* peeraddr);
////********新连接**************************************************************  



// 
#endif