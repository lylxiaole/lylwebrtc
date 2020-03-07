/*
 *  Copyright 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "totaltest.h"



#include "rtc_base/win32socketinit.h" 
#include "rtc_base/thread.h" 
#include "api/scoped_refptr.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"


#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "pc/mediastreamobserver.h" 
#include "pc/test/fakertccertificategenerator.h"
#include "media/engine/webrtcvideocapturerfactory.h"


using namespace std;
using namespace lylwebrtc;
using namespace webrtc;
using namespace rtc;
std::unique_ptr<rtc::Thread> worker_thread;
std::unique_ptr<rtc::Thread> signaling_thread;
std::unique_ptr<rtc::Thread> network_thread;
//********caller**************************************************************
bool callerIsCompletedSdpChange = false;
rtc::scoped_refptr <lylwebrtc::lylPeerConnection> caller = nullptr;
rtc::scoped_refptr <lylwebrtc::lylPeerConnection> callee = nullptr;
lylDataChannel* callerdataChannel = nullptr;

void InitializeCaller()
{
	caller = rtc::scoped_refptr<lylPeerConnection>(new rtc::RefCountedObject<lylPeerConnection>(worker_thread.get(), signaling_thread.get(), network_thread.get()));

	caller->onCreateSdpSuccess = caller_onCreateSdpSuccess;
	caller->onIceCandidateHandle = caller_onIceCandidate;
	caller->onCreateSdpFailure = caller_onCreateSdpFailure;

	caller->onSignalingStateHandle = caller_onSignalingState;
	caller->onIceGatheringStateHandle = caller_onIceGatheringState;
	caller->onRemoveStreamHandle = caller_onRemoveStream;
	caller->onRemoveTrackHandle = caller_onRemoveTrack;
	caller->onDataChannelHandle = caller_onDataChannel;
	caller->onAddTrackHandle = caller_onAddTrack;
	caller->onAddStreamHandle = caller_onAddStream;
	caller->onIceConnectionStateHandle = caller_onIceConnectionState;
	caller->onLocalFrameHandle = caller_onLocalFrameData;

}



//创建一个链接工厂类
//
void InitializeThread() {
#if defined(WEBRTC_WIN)
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
#endif  

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


void caller_onGetPeerConnectionStats(const webrtc::RTCStatsReport* state)
{

}
void CallerCreateOffer()
{
	caller->CreateOffer();
}

void caller_onSignalingState(webrtc::PeerConnectionInterface::SignalingState state) {
	std::cout << "caller_onSignalingState\n";
}

void caller_onIceGatheringState(webrtc::PeerConnectionInterface::IceGatheringState state) {
	std::cout << "caller_onIceGatheringState\n";
}

void caller_onRemoveStream(char* streamLabel) {
	std::cout << "caller_onrRemoveStream\n";
}

void caller_onRemoveTrack(char* streamLabel, char* trackId) {
	std::cout << "caller_onRemoveTrack\n";
}

void caller_onDataChannel(char* data_channelId) {
	std::cout << "caller_onDataChannel\n";
}
void caller_onChannelStateChanged(webrtc::DataChannelInterface::DataState state, char* channelId) {
	std::cout << "caller_onChannelStateChanged\n";
}
void caller_onChannelMessage(const char* buffer, int length, bool binary, char* channelId) {
	std::cout << "caller_onChannelMessage\n";
}

void caller_onAddTrack(char* streamLabel, char* trackId, char* trackType) {
	std::cout << "caller_onAddTrack\n";
	auto videotrack = caller->findVideoTrack(trackId);
	if (videotrack)
	{
		videotrack->onFrameHandle = caller_onRemoteFrameData;
	}
}

void caller_onAddStream(char* streamLabel) {
	std::cout << "caller_onAddStream\n";
}

void caller_onIceConnectionState(webrtc::PeerConnectionInterface::IceConnectionState state) {
	if (state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed)
	{
		caller->DeletePeerConnection();
	}
	std::cout << "caller_onIceConnectionState\n";
}

void caller_onIceCandidate(char* candidate, char* sdp_mid, int sdp_mline_index) {
	std::cout << "caller_onIceConnectionState\n";
	callee->AddRemoteIceCandidate(candidate, sdp_mid, sdp_mline_index);
	dealIceCandidate();
}

void caller_onRemoteFrameData(unsigned char* imgdata, size_t datalength, int width, int height, const char* trackId) {
	//auto stats = caller->GetPeerStats();
	//if (stats)
	//{ 
	//	std::cout << "caller_Stats:" << stats->ToJson()<<" \n";
	//}
}
// uint8_t* imgdata, size_t datalength, int width, int height, const char*
// trackId, const void* rtcConnectAddr
void caller_onLocalFrameData(unsigned char* imgdata, size_t datalength, int width, int height, const char* trackId) {
	//std::cout << datalength << "\n"; 

}

void caller_onCreateSdpSuccess(const char* sdp, const char* type) {
	std::cout << sdp;
	std::cout << "!\n";
	std::cout << type;

	callee->SetRemoteSdp(sdp, type);
	dealIceCandidate();
}

void caller_onCreateSdpFailure(const char* erro) {
	std::cout << erro;
	std::cout << "!\n";
}


//********callee**************************************************************
bool calleeIsCompletedSdpChange = false;

void InitializeCallee()
{
	callee = rtc::scoped_refptr<lylPeerConnection>(new rtc::RefCountedObject<lylPeerConnection>(worker_thread.get(), signaling_thread.get(), network_thread.get()));
	callee->onCreateSdpSuccess = callee_onCreateSdpSuccess;
	callee->onIceCandidateHandle = callee_onIceCandidate;
	callee->onCreateSdpFailure = callee_onCreateSdpFailure;
	callee->onSignalingStateHandle = callee_onSignalingState;
	callee->onIceGatheringStateHandle = callee_onIceGatheringState;
	callee->onRemoveStreamHandle = callee_onRemoveStream;
	callee->onRemoveTrackHandle = callee_onRemoveTrack;
	callee->onDataChannelHandle = callee_onDataChannel;
	callee->onAddTrackHandle = callee_onAddTrack;
	callee->onAddStreamHandle = callee_onAddStream;
	callee->onIceConnectionStateHandle = callee_onIceConnectionState;
	callee->onLocalFrameHandle = callee_onLocalFrameData;

}


void callee_onGetPeerConnectionStats(const webrtc::RTCStatsReport* state)
{

}

void callee_onSignalingState(webrtc::PeerConnectionInterface::SignalingState state) {
	std::cout << "callee_onSignalingState\n";
}

void callee_onIceGatheringState(webrtc::PeerConnectionInterface::IceGatheringState state) {
	std::cout << "callee_onIceGatheringState\n";
}

void callee_onRemoveStream(char* streamLabel) {
	std::cout << "callee_onrRemoveStream\n";
}

void callee_onRemoveTrack(char* streamLabel, char* trackId) {
	std::cout << "callee_onRemoveTrack\n";
}

void callee_onDataChannel(char* data_channelId) {
	std::cout << "callee_onDataChannel\n";
}
void callee_onChannelStateChanged(webrtc::DataChannelInterface::DataState state, char* channelId) {
	std::cout << "callee_onChannelStateChanged\n";
}
void callee_onChannelMessage(const char* buffer, int length, bool binary, char* channelId) {
	std::cout << "callee_onChannelMessage\n";
}

void callee_onAddTrack(char* streamLabel, char* trackId, char* trackType) {
	std::cout << "callee_onAddTrack\n";
	auto videotrack = caller->findVideoTrack(trackId);
	if (videotrack)
	{
		videotrack->onFrameHandle = callee_onRemoteFrameData;
	}
}

void callee_onAddStream(char* streamLabel) {
	std::cout << "callee_onAddStream\n";
}

void callee_onIceConnectionState(webrtc::PeerConnectionInterface::IceConnectionState state) {
	if (state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed)
	{
		callee->DeletePeerConnection();
	}
	std::cout << "callee_onIceConnectionState\n";
}

void callee_onIceCandidate(char* candidate, char* sdp_mid, int sdp_mline_index) {
	std::cout << "callee_onIceConnectionState\n";
	caller->AddRemoteIceCandidate(candidate, sdp_mid, sdp_mline_index);
	dealIceCandidate();
}

void callee_onRemoteFrameData(unsigned char* imgdata, size_t datalength, int width, int height, const char* trackId) {

	//auto stats = callee->GetPeerStats();
	//if (stats)
	//{
	//	std::cout << "caller_Stats:" << stats->ToJson() << " \n";
	//}
}
// uint8_t* imgdata, size_t datalength, int width, int height, const char*
// trackId, const void* rtcConnectAddr
void callee_onLocalFrameData(unsigned char* imgdata, size_t datalength, int width, int height, const char* trackId) {
	//std::cout << datalength << "\n";  

}

void callee_onCreateSdpSuccess(const char* sdp, const char* type) {
	std::cout << sdp;
	std::cout << "!\n";
	std::cout << type;

	caller->SetRemoteSdp(sdp, type);
	dealIceCandidate();
}

void callee_onCreateSdpFailure(const char* erro) {
	std::cout << erro;
	std::cout << "!\n";
}


//**********************************************************************

void dealIceCandidate()
{
	if (caller&&caller->isCompletedSdpChange&&callee&&callee->isCompletedSdpChange)
	{
		caller->DealCacheCandidates();
		callee->DealCacheCandidates();
	}
}


int main(int argc, char* argv[]) {

#if defined(WEBRTC_WIN)
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
#endif 
	rtc::InitializeSSL();
	InitializeThread();
	/********************************************/
	InitializeCaller();
	InitializeCallee();

	webrtc::PeerConnectionInterface::IceServer server;
	//server.uri = "turn:192.168.123.222:65001?transport=udp";
	server.uri = "turn:212.64.71.83:3478?transport=tcp"; 
	server.username = "123";
	server.password = "123";
	caller->config.servers.push_back(server);
	caller->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kRelay;
	callee->config.servers.push_back(server);
	callee->config.type = webrtc::PeerConnectionInterface::IceTransportsType::kRelay;


	caller->isAudio = true;
	callee->isAudio = true;
	caller->isDesktop = true;
	callee->isDesktop = true;

	CallerCreateOffer();


	int i = 0;
	std::cin >> i;
	std::cout << "Hello World!\n";

	caller->DeletePeerConnection();

	std::cin >> i;
	return 1;
}
