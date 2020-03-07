#include <memory> 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>  // NOLINT
#include <memory>
#include <vector>


#include "rtc_base/thread.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/json.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/bind.h"
#include "rtc_base/win32socketinit.h"
#include "api/create_peerconnection_factory.h"

#include "api/scoped_refptr.h"
#include "rtc_base/refcountedobject.h"
#include "globalHraders.h"  
#include "src/lylPeerConnection.h" 

void InitializeCaller();

void InitializeThread();

void CallerCreateOffer();

void caller_onSignalingState(webrtc::PeerConnectionInterface::SignalingState state);

void caller_onIceGatheringState(webrtc::PeerConnectionInterface::IceGatheringState state);

void caller_onRemoveStream(char * streamLabel);

void caller_onRemoveTrack(char * streamLabel, char * trackId);

void caller_onDataChannel(char* data_channelId);

void caller_onChannelStateChanged(webrtc::DataChannelInterface::DataState state, char* channelId);

void caller_onChannelMessage(const char * buffer, int length, bool binary, char* channelId);

void caller_onAddTrack(char * streamLabel, char * trackId, char * trackType);

void caller_onAddStream(char * streamLabel);

void caller_onIceConnectionState(webrtc::PeerConnectionInterface::IceConnectionState state);

void caller_onIceCandidate(char * candidate, char * sdp_mid, int sdp_mline_index);

void caller_onRemoteFrameData(unsigned char * imgdata, size_t datalength, int width, int height, const char * trackId);

void caller_onLocalFrameData(unsigned char * imgdata, size_t datalength, int width, int height, const char * trackId);

void caller_onCreateSdpSuccess(const char * sdp, const char * type);

void caller_onCreateSdpFailure(const char * erro);

void InitializeCallee();

void callee_onSignalingState(webrtc::PeerConnectionInterface::SignalingState state);

void callee_onIceGatheringState(webrtc::PeerConnectionInterface::IceGatheringState state);


void callee_onRemoveStream(char * streamLabel);

void callee_onRemoveTrack(char * streamLabel, char * trackId);

void callee_onDataChannel(char* data_channelId);

void callee_onChannelStateChanged(webrtc::DataChannelInterface::DataState state, char* channelId);

void callee_onChannelMessage(const char * buffer, int length, bool binary, char* channelId);

void callee_onAddTrack(char * streamLabel, char * trackId, char * trackType);

void callee_onAddStream(char * streamLabel);

void callee_onIceConnectionState(webrtc::PeerConnectionInterface::IceConnectionState state);

void callee_onIceCandidate(char * candidate, char * sdp_mid, int sdp_mline_index);

void callee_onRemoteFrameData(unsigned char * imgdata, size_t datalength, int width, int height, const char * trackId);

void callee_onLocalFrameData(unsigned char * imgdata, size_t datalength, int width, int height, const char * trackId);

void callee_onCreateSdpSuccess(const char * sdp, const char * type);

void callee_onCreateSdpFailure(const char * erro);

void dealIceCandidate();



