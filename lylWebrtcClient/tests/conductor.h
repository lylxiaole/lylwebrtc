/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h" 

namespace webrtc {
	class VideoCaptureModule;
}  // namespace webrtc

namespace cricket {
	class VideoRenderer;
}  // namespace cricket

class Conductor : public webrtc::PeerConnectionObserver, public webrtc::CreateSessionDescriptionObserver {
	
	typedef void(*_onSendMessage)(std::string* message);

public:
	enum CallbackID {
		MEDIA_CHANNELS_INITIALIZED = 1,
		PEER_CONNECTION_CLOSED,
		SEND_MESSAGE_TO_PEER,
		NEW_TRACK_ADDED,
		TRACK_REMOVED,
	};

	Conductor();
	bool Initialize_thread();
	~Conductor();

	_onSendMessage OnSendMessage=nullptr;
	bool connection_active() const;
	void Close();

	bool InitializePeerConnection();
	void CreateAudioDeviceModule_w();
	bool ReinitializePeerConnectionForLoopback();
	bool CreatePeerConnection(bool dtls);
	void DeletePeerConnection();
	void EnsureStreamingUI();
	void AddTracks();
	std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice();
	void ConnectToPeer(int peer_id);
	void OnMessageFromPeer(int peer_id, const std::string& message);

	void DealCacheCandidates();
	bool isCompletedSdpChange = false;
	//
	// PeerConnectionObserver implementation.
	//
protected:
	void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {};
	void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&streams) override;
	void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
	void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {};
	void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {};
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
	void OnIceConnectionReceivingChange(bool receiving) override {}

	//
	// PeerConnectionClientObserver implementation.
	// 
	void OnSignedIn(); 
	void OnDisconnected(); 
	void OnPeerConnected(int id, const std::string& name); 
	void OnPeerDisconnected(int id); 
	void OnMessageSent(int err); 
	void OnServerConnectionFailure(); 
	void DisconnectFromServer(); 
	void DisconnectFromCurrentPeer(); 
	void UIThreadCallback(int msg_id, void* data); 

	// CreateSessionDescriptionObserver implementation.
	void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
	void OnFailure(webrtc::RTCError error) override;


protected:
	// Send a message to the remote peer.
	void SendMessage(const std::string& json_object);

	int peer_id_;
	bool loopback_;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
	//PeerConnectionClient* client_;
	//MainWindow* main_wnd_;
	std::deque<std::string*> pending_messages_;
	std::string server_;

private:

	std::unique_ptr<rtc::Thread> worker_thread = nullptr;
	std::unique_ptr<rtc::Thread> signaling_thread = nullptr;
	std::unique_ptr<rtc::Thread> network_thread = nullptr;
	rtc::scoped_refptr<webrtc::AudioDeviceModule>  audioDeviceModule_ = nullptr;

	std::vector<webrtc::IceCandidateInterface*> cacheCandidates_;



};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
