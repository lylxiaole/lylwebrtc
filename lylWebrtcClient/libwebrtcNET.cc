
#include "libwebrtcNET.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"

#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "pc/mediastreamobserver.h" 
#include "pc/test/fakertccertificategenerator.h"
#include "media/engine/webrtcvideocapturerfactory.h"

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/json.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/bind.h"
#include "rtc_base/win32socketinit.h"

#include "media/base/fakemediaengine.h"

#include <memory>

#include "lylDesktopCapturer.h"

using namespace lylwebrtc;
using namespace webrtc;


rtc::scoped_refptr <lylPeerConnection> findPeerConnection(void* paddr)
{
	for (size_t i = 0; i < peer_connections.size(); i++)
	{
		if (paddr == peer_connections[i].get())
		{
			return peer_connections[i];
		}
	}
	return nullptr;
}



//创建一个链接工厂类
//
void Initialize() {
#if defined(WEBRTC_WIN)
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
#endif  
	rtc::InitializeSSL();
	//	rtc::CleanupSSL();
	/***/
	videoDeviceModule_ = webrtc::VideoCaptureFactory::CreateDeviceInfo();
	audioDeviceModule_ = webrtc::AudioDeviceModule::Create(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);
#if defined(WEBRTC_WIN)
	std::unique_ptr<rtc::WinsockInitializer> winsock_init_ = absl::make_unique<rtc::WinsockInitializer>();
#endif 

	if (!worker_thread) {
		worker_thread = rtc::Thread::Create();
		worker_thread->SetName("worker_thread", nullptr);
		RTC_CHECK(worker_thread->Start()) << "Failed to start thread";
	}
	if (!signaling_thread) {
		signaling_thread = rtc::Thread::Create();
		signaling_thread->SetName("signaling_thread", nullptr);
		RTC_CHECK(signaling_thread->Start()) << "Failed to start thread";
	}
	if (!network_thread) {
		network_thread = rtc::Thread::CreateWithSocketServer();
		network_thread->SetName("network_thread", nullptr);
		RTC_CHECK(network_thread->Start()) << "Failed to start thread";
	}
}

void RealseAll()
{
	rtc::CleanupSSL();
	rtc::ThreadManager::Instance()->SetCurrentThread(NULL);
	// 
	if (worker_thread.get() != nullptr) {
		worker_thread->Stop();
		worker_thread.release();
		worker_thread.reset(nullptr);
	}
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 8";
	if (network_thread.get() != nullptr) {
		network_thread->Stop();
		network_thread.release();
		network_thread.reset(nullptr);
	}
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 10";
	if (signaling_thread) {
		signaling_thread->Stop();
		signaling_thread.release();
		signaling_thread = nullptr;
	}
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 9";
}

void* CreatePeerConnection() {
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********libwebrctNET";
	auto newpeer = rtc::scoped_refptr<lylPeerConnection>(new rtc::RefCountedObject<lylPeerConnection>(worker_thread.get(), signaling_thread.get(), network_thread.get()));
	peer_connections.push_back(newpeer);
	return newpeer.get();
}

void CloseConnection(void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********before delete connection:";
		peer_connections.erase(
			std::remove_if(
				peer_connections.begin(), peer_connections.end(),
				[peer_connection_](const scoped_refptr<lylPeerConnection> pc_) {
			return pc_ == peer_connection_;
		}), peer_connections.end());

		peer_connection_->DeletePeerConnection();
		//delete peer_connection_ .get();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********after delete connection:";

	}
}

////注册一个Ice服务,在Initialize之前调用
void RegisterIceServer(const char uri[kMaxStringLength], const char username[kMaxStringLength], const char password[kMaxStringLength], void* peeraddr) {
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********init config";
	webrtc::PeerConnectionInterface::IceServer server = webrtc::PeerConnectionInterface::IceServer();

	server.uri = std::string(uri);
	server.username = std::string(username);
	server.password = std::string(password);
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********search conection";
	auto peerconnection = findPeerConnection(peeraddr);

	if (peerconnection)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********before RegisterIceServer";
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********servers size:" << peerconnection->config.servers.size();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********new server:" << &(server);
		peerconnection->config.servers.push_back(server);
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********after RegisterIceServer";
	}
}


void SetPeerConnectionConfig(int icetype, void* peeraddr)
{
	auto peerconnection = findPeerConnection(peeraddr);
	if (peerconnection)
	{
		if (icetype == 0)
		{
			peerconnection->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kNone;
		}
		else if (icetype == 1)
		{
			peerconnection->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kRelay;
		}
		else if (icetype == 2)
		{
			peerconnection->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kNoHost;
		}
		else if (icetype == 3)
		{
			peerconnection->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kAll;
		}
	}
}

void SetTrackDevice(bool isVedio, int vedioIndex, bool isAudio, int recordAudioIndex, bool isDesktop, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->isVedio = isVedio;
		peer_connection_->vedioIndex = vedioIndex;
		peer_connection_->isAudio = isAudio;
		peer_connection_->recordAudioIndex = recordAudioIndex;
		peer_connection_->isDesktop = isDesktop;
	}
}

char* CreateDataChannel(char* channelLabel, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->CreateDataChannel(channelLabel);
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********return channelLabel:" << channelLabel;
		return (char*)channelLabel;
	}
	return nullptr;
}

//获取视频输入设备的数量
int32_t GetVideoDeviceNumber()
{
	return videoDeviceModule_->NumberOfDevices();
}
//根据Index获取视频输出设备信息
void GetVideoDevice(int32_t index, VedioDevice& refdeviceInfo) {
	videoDeviceModule_->GetDeviceName(index, refdeviceInfo.deviceNameUTF8, refdeviceInfo.deviceNameLength, refdeviceInfo.deviceUniqueIdUTF8, refdeviceInfo.deviceUniqueIdUTF8Length, refdeviceInfo.productUniqueIdUTF8, refdeviceInfo.productUniqueIdUTF8Length);
}
//获取音频输出设备的数量
int16_t GetPlayoutAudioDeviceNumber() {
	return	audioDeviceModule_->PlayoutDevices();
}
//根据Index获取音频输出设备信息
void GetPlayoutAudioDevice(int32_t index, AudioDeive& refAudioDevice) {
	audioDeviceModule_->PlayoutDeviceName(index, refAudioDevice.name, refAudioDevice.guid);
}
//获取音频输入设备的数量
int16_t GetRecordingAudioDeviceNumber() {
	return audioDeviceModule_->RecordingDevices();
}
//根据Index获取音频输入设备信息
void GetRecordingAudioDevice(int32_t index, AudioDeive& refAudioDevice) {
	audioDeviceModule_->RecordingDeviceName(index, refAudioDevice.name, refAudioDevice.guid);
}
//
void SetPlayoutAudioDevice(uint32_t playoutAudioIndex)
{
	audioDeviceModule_->SetPlayoutDevice(playoutAudioIndex);
}
////*************信令相关*********************************************************
void CreateOffer(_sdpCreateSuccess onsuccess, _sdpCreateFailure onfailure, void* peeraddr) {
	RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** -3";
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** -2";
		peer_connection_->onCreateSdpSuccess = onsuccess;
		peer_connection_->onCreateSdpFailure = onfailure;
		peer_connection_->CreateOffer();
	}
}

void AddCandidate(char* candiate, char* mid, int midx, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->AddRemoteIceCandidate(candiate, mid, midx);
	}
}
//**设置远程发来的SDP，如果发来的是offer，本地会创建answer并触发事件
void SetRemoteDescription(const char* offer_sdp, const char* type, _sdpCreateSuccess onCreateSdpSuccess, _sdpCreateFailure onCreateSdpFailure, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onCreateSdpSuccess = onCreateSdpSuccess;
		peer_connection_->onCreateSdpFailure = onCreateSdpFailure;
		peer_connection_->SetRemoteSdp(offer_sdp, type);
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********" << offer_sdp;
	}
}

void DealCacheIceCandidate(void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->DealCacheCandidates();
	}
}

////********管道**************************************************************
void SendChannelData(char* channelLabel, const char* data, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);

	if (peer_connection_)
	{
		auto channel = peer_connection_->findDataChannel(channelLabel);
		if (channel)
		{
			//RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** send data length:" << strlen(data) << ",data:" << data;
			/*char* d = nullptr;
			memset(d, 0, strlen(data));
			memcpy(d, data, strlen(data));*/
			channel->Send(data, strlen(data), false);
			//delete d;
		}
		else
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** not find channel";
		}
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** not find peerconnection";
	}
}

//
void RemoveDataChannel(char* channelLabel, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->removeDataChannel(channelLabel);
	}
}
void SetDataChannelEvent(_onChannelStateChanged onStateChangeHandle, _onChannelMessage onMessageHandle, char* channelLabel, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		auto channel = peer_connection_->findDataChannel(channelLabel);
		if (channel)
		{
			channel->onMessageHandle = onMessageHandle;
			channel->onStateChangeHandle = onStateChangeHandle;
		}
	}
}

void  RemoveStream(char* streamId, void* peeraddr)
{
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->RemoveStream(streamId);
	}
}

////********Peerconnection事件**************************************************************
void SetListenerSignalingState(_onSignalingState handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onSignalingStateHandle = handle;
	}
}
//
void SetListenerIceGatheringState(_onIceGatheringState handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onIceGatheringStateHandle = handle;
	}
}
//
void SetListenerRemoveStream(_onRemoveStream handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onRemoveStreamHandle = handle;
	}
}
//
void SetListenerRemoveTrack(_onRemoveTrack handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onRemoveTrackHandle = handle;
	}
}
//
void SetListenerAddTrack(_onAddTrack handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onAddTrackHandle = handle;
	}
}
//
void SetListenerAddStream(_onAddStream handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onAddStreamHandle = handle;
	}
}
//
void SetListenerIceConnectionState(_onIceConnectionState handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onIceConnectionStateHandle = handle;
	}
}
//
void SetListenerIceCandidate(_onIceCandidate onIceCandidate, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********SetListenerIceCandidate:";
		peer_connection_->onIceCandidateHandle = onIceCandidate;
	}
}
//
void SetListenonLocalFrame(_onFrame onFrameMethod, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onLocalFrameHandle = onFrameMethod;
	}
}
//
void SetListenonRemoteFrame(_onFrame onFrameMethod, char* trackId, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		auto track = peer_connection_->findVideoTrack(trackId);
		if (track)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********SetListenonRemoteFrame,trackId=" << trackId;
			track->onFrameHandle = onFrameMethod;
		}
	}
}
//
void SetListenerOnNewDataChannel(_onDataChannel handle, void* peeraddr) {
	auto peer_connection_ = findPeerConnection(peeraddr);
	if (peer_connection_)
	{
		peer_connection_->onDataChannelHandle = handle;
	}
}
////********新连接**************************************************************




