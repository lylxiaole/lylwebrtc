#include "lylPeerConnection.h"
#include "lylDesktopCapturer.h"

#include "lylMediaConstraints.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"

#include "modules/video_capture/video_capture_factory.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "pc/sessiondescription.h"
#include "pc/datachannel.h"

#include "rtc_base/ssladapter.h"  
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

#include "rtc_base/bind.h"

namespace lylwebrtc
{
	lylPeerConnection::lylPeerConnection(Thread* worker_thread, Thread* signaling_thread, Thread* network_thread) :
		worker_thread_(worker_thread), signaling_thread_(signaling_thread), network_thread_(network_thread), config()
	{
		sdpOptions_.offer_to_receive_audio = 1;
		sdpOptions_.offer_to_receive_video = 1;
		sdpOptions_.use_rtp_mux = true;
		sdpOptions_.ice_restart = false;
	}
	//***************************************PeerConnection methods 
	const RTCStatsReport* lylPeerConnection::GetPeerStats()
	{
		if (peer_connection_)
		{ 
			rtc::scoped_refptr<RTCStatsObtainer> stats_obtainer = RTCStatsObtainer::Create(); 
			peer_connection_->GetStats(stats_obtainer);  
			return stats_obtainer->report().get();
		}
		return nullptr;
	}


	bool lylPeerConnection::InitializePeerConnection()
	{
		this->isCompletedSdpChange = false;
		RTC_DCHECK(!peer_connection_);
		if (!peer_connection_factory_)
		{ 
			peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
				this->network_thread_, this->worker_thread_,
				this->signaling_thread_, nullptr /* default_adm */,
				webrtc::CreateBuiltinAudioEncoderFactory(),
				webrtc::CreateBuiltinAudioDecoderFactory(),
				webrtc::CreateBuiltinVideoEncoderFactory(),
				webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */, nullptr /* audio_processing */);
		}
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 2";
		if (!CreatePeerConnection(/*dtls=*/true)) {
			DeletePeerConnection();
		}
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 3";
		DealCacheDataChannels();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 4";
		AddTracks();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 5";
		return peer_connection_ != nullptr;
	}
	bool lylPeerConnection::CreatePeerConnection(bool dtls)
	{
		RTC_DCHECK(peer_connection_factory_);
		RTC_DCHECK(!peer_connection_);

		config.sdp_semantics = webrtc::SdpSemantics::kPlanB;
		config.enable_dtls_srtp = dtls;

		//
		//webrtc::PeerConnectionInterface::IceServer server; 
		//server.uri = "turn:192.168.123.52:3478?transport=udp";
		//server.username = "123";
		//server.password = "123"; 
		//config.servers.push_back(server);
		//
		//config.type = webrtc::PeerConnectionInterface::IceTransportsType::kAll;

		//webrtc::MediaConstraintsInterface	 
		//auto pcConstraints = new  webrtc::MediaConstraintsInterface();
		//pcConstraints.mandatory.add(new MediaConstraints.KeyValuePair(CPU_OVERUSE_DETECTION_CONSTRANIT, "true")
		peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);
		 
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********" << config.servers[0].uri;
		return peer_connection_ != nullptr;
	}
	void lylPeerConnection::DeletePeerConnection()
	{
		/*signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::DeletePeerConnection_a, this));
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::DeletePeerConnection_b, this));*/
			DeletePeerConnection_a();
			DeletePeerConnection_b();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 6-1";
	}

	void lylPeerConnection::DeletePeerConnection_b()
	{
		if (peer_connection_&&peer_connection_->ice_connection_state() == PeerConnectionInterface::IceConnectionState::kIceConnectionClosed)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 1";
			for (auto channel : dataChannels)
			{
				channel->Close();
			}
			dataChannels.clear();
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 2";
			//
			if (localStream_)
			{
				for (auto track : localStream_->GetAudioTracks()) {
					localStream_->RemoveTrack(track.get());
				}
				for (auto track : localStream_->GetVideoTracks()) {
					localStream_->RemoveTrack(track);
				}
				localStream_ = nullptr;
			}
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 3";
			//
			for (auto stream : remoteStreams_)
			{
				for (auto track : stream->GetAudioTracks()) {
					stream->RemoveTrack(track.get());
				}
				for (auto track : stream->GetVideoTracks()) {
					stream->RemoveTrack(track);
				}
				peer_connection_->RemoveStream(stream);
			}
			remoteStreams_.clear();
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 4";
			//
			for (auto track : peer_connection_->GetSenders())
			{
				peer_connection_->RemoveTrack(track);
			}
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 5";
			//
			if (localVideoRender_)
			{
				localVideoRender_->RemoveSink();
				localVideoRender_ = nullptr;
			}
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 6";
			//
			for (auto rerender : remoteVideoRenders)
			{
				rerender->RemoveSink();
			}
			remoteVideoRenders.clear();
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 7";
			//
			if (peer_connection_)
			{
				peer_connection_.release();
				peer_connection_ = nullptr;
			}
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 8";
			if (peer_connection_factory_)
			{
				peer_connection_factory_.release();
				peer_connection_factory_ = nullptr;
			}
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 9";
		}

	}

	void lylPeerConnection::DeletePeerConnection_a()
	{
		if (peer_connection_&&peer_connection_->ice_connection_state() != PeerConnectionInterface::IceConnectionState::kIceConnectionClosed)
		{
			peer_connection_->Close();
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 0";
		}

	/*	this->network_thread_ = nullptr;
		this->signaling_thread_ = nullptr;
		this->worker_thread_ = nullptr;*/
	}

	void lylPeerConnection::AddTracks()
	{
		if (!peer_connection_->GetSenders().empty())
		{
			return;  // Already added tracks.
		}
		char streamId[64] = { 0 };
		newGUID(streamId);
		auto localStream = peer_connection_factory_->CreateLocalMediaStream({ streamId });

		if (isVedio)
		{
			rtc::scoped_refptr<webrtc::VideoTrackInterface> videotrack = CreateVideoTrack(); 
			localVideoRender_ = new lylwebrtc::lylVideoRender(videotrack->id().data(), streamId, videotrack);
			videotrack->AddOrUpdateSink(localVideoRender_, rtc::VideoSinkWants());
			localVideoRender_->onFrameHandle = this->onLocalFrameHandle;
			localStream->AddTrack(videotrack);
		}

		if (isDesktop)
		{
			rtc::scoped_refptr<webrtc::VideoTrackInterface> videotrack = CreateDesktopTrack();
			localVideoRender_ = new lylwebrtc::lylVideoRender(videotrack->id().data(), streamId, videotrack);
			videotrack->AddOrUpdateSink(localVideoRender_, rtc::VideoSinkWants());
			localVideoRender_->onFrameHandle = this->onLocalFrameHandle;
			localStream->AddTrack(videotrack);
		}
		if (isAudio)
		{
			auto audio_track = this->CreateAudioTrack();
			//auto result_or_error = peer_connection_->AddTrack(audio_track, { kStreamId });
			localStream->AddTrack(audio_track);
		}
		peer_connection_->AddStream(localStream);
		localStream_ = localStream; 
		 
		///*码率设置*/
		//RtpTransceiverInit init;
		//init.direction = RtpTransceiverDirection::kSendRecv;
		//init.stream_ids = { streamId };
		//RtpEncodingParameters param1;
		//param1.max_framerate = 60;
		//param1.min_bitrate_bps=
		//init.send_encodings.push_back(param1)
		//peer_connection_->AddTransceiver(cricket::MEDIA_TYPE_VIDEO, init,); 

	}

	std::unique_ptr<cricket::VideoCapturer> lylPeerConnection::OpenVideoCaptureDevice()
	{
		std::vector<std::string> device_names;
		{
			std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
			if (!info) {
				return nullptr;
			}
			int num_devices = info->NumberOfDevices();
			for (int i = 0; i < num_devices; ++i)
			{
				const uint32_t kSize = 256;
				char name[kSize] = { 0 };
				char id[kSize] = { 0 };
				if (info->GetDeviceName(i, name, kSize, id, kSize) != -1)
				{
					device_names.push_back(name);
				}
			}
		}

		cricket::WebRtcVideoDeviceCapturerFactory factory;
		std::unique_ptr<cricket::VideoCapturer> capturer;
		for (const auto& name : device_names)
		{
			capturer = factory.Create(cricket::Device(name, 0));
			if (capturer)
			{
				break;
			}
		}
		return capturer;
	}
	rtc::scoped_refptr<webrtc::VideoTrackInterface> lylPeerConnection::CreateVideoTrack()
	{
		std::unique_ptr<cricket::VideoCapturer> video_device = OpenVideoCaptureDevice();
		if (video_device)
		{
			scoped_refptr<lylMediaConstraints> mediaconstarints = lylMediaConstraints::Create();
			mediaconstarints->AddOptionalConstraint(lylMediaConstraints::kCpuOveruseDetection, "true");
			mediaconstarints->AddOptionalConstraint(lylMediaConstraints::kMaxFrameRate, "25");

			char newId[64] = { 0 };
			newGUID(newId);

			rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
				peer_connection_factory_->CreateVideoTrack(newId, peer_connection_factory_->CreateVideoSource(std::move(video_device), mediaconstarints)));
			return video_track_;
		}
		return NULL;
	}
	rtc::scoped_refptr<webrtc::VideoTrackInterface> lylPeerConnection::CreateDesktopTrack()
	{
		webrtc::DesktopCaptureOptions options = webrtc::DesktopCaptureOptions::CreateDefault();
		options.set_allow_directx_capturer(true);
		options.set_disable_effects(true); 

		lylwebrtc::lylDesktopCapturer* desktopCapturer = new lylwebrtc::lylDesktopCapturer(webrtc::DesktopCapturer::CreateScreenCapturer(options));
	 
		std::unique_ptr<cricket::VideoCapturer> videoCapturer = std::unique_ptr<cricket::VideoCapturer>(desktopCapturer);
		scoped_refptr<lylMediaConstraints> mediaconstarints = lylMediaConstraints::Create();
		mediaconstarints->AddOptionalConstraint(lylMediaConstraints::kCpuOveruseDetection, "true");
		mediaconstarints->AddOptionalConstraint(lylMediaConstraints::kMaxFrameRate, "20");
		 
		char newId[64] = { 0 };
		newGUID(newId);
		rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
			peer_connection_factory_->CreateVideoTrack(newId, peer_connection_factory_->CreateVideoSource(std::move(videoCapturer), mediaconstarints)));
		 

		return video_track_;
	}
	rtc::scoped_refptr<webrtc::AudioTrackInterface> lylPeerConnection::CreateAudioTrack()
	{
		char newId[64] = { 0 };
		newGUID(newId);
		rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
			peer_connection_factory_->CreateAudioTrack(newId, peer_connection_factory_->CreateAudioSource(cricket::AudioOptions())));
		return audio_track;
	}

	void lylPeerConnection::DealSdpOtherInfo(webrtc::SessionDescriptionInterface* desc)
	{
		cricket::ContentDescription* content_desc = desc->description()->GetContentDescriptionByName("video");
		cricket::MediaContentDescription* media_content_desc = (cricket::MediaContentDescription*)content_desc;
		if (media_content_desc)
		{
			//media_content_desc->set_bandwidth(512 * 1024);//视频带宽512KB  
			 
		}
	}

	void lylPeerConnection::CreateDataChannel(char* label)
	{
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::CreateDataChannel_s, this, label));
	}
	void lylPeerConnection::CreateDataChannel_s(char* label)
	{
		//rtc::scoped_refptr<webrtc::DataChannelInterface> rtc_data_channel = peer_connection_->CreateDataChannel(label, nullptr);
		lylDataChannel* channelObserver = new lylDataChannel(label);
		dataChannels.push_back(channelObserver);

		//DataChannelInit config;
		//config.ordered = true;
		//config.reliable = true;
		//config.maxRetransmitTime = -1;
		//config.maxRetransmits = -1;
		//config.protocol = "sctp";  // sctp | quic
		//config.negotiated = false;
		//config.id =0;   
		//for (size_t i = 0; i < this->channelLabels.size(); i++)
		//{
		//	auto label = this->channelLabels[i];
		//	//
		//	/*const std::string label = newGUID();*/
		//	rtc::scoped_refptr<webrtc::DataChannelInterface> rtc_data_channel = peer_connection_->CreateDataChannel(label, nullptr);
		//	lylDataChannel* channelObserver = new lylDataChannel(rtc_data_channel, label.data());
		//	dataChannels.push_back(channelObserver);
		//}
	}
	void lylPeerConnection::DealCacheDataChannels()
	{
		//webrtc::DataChannelInit channelConfig;
	   // channelConfig. 
  //  
		for (size_t i = 0; i < this->dataChannels.size(); i++)
		{
			auto channelObserver = this->dataChannels[i];
			//
			/*const std::string label = newGUID();*/
			rtc::scoped_refptr<webrtc::DataChannelInterface> rtc_data_channel = peer_connection_->CreateDataChannel(channelObserver->label, nullptr);
			channelObserver->RegisterDataChannel(rtc_data_channel.get());
		}
	}

	void lylPeerConnection::CreateOffer()
	{
		signaling_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::InitializePeerConnection, this));
		//InitializePeerConnection();
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** -1" << Thread::Current();
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::CreateOffer_s, this));
		//CreateOffer_s();
	}
	void lylPeerConnection::CreateOffer_s()
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 0";
		peer_connection_->CreateOffer(this, sdpOptions_);
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** 1";
	}

	void lylPeerConnection::SetRemoteSdp(const char* sdp, const char* type)
	{
		if (!peer_connection_.get())
		{
			signaling_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::InitializePeerConnection, this));
			//InitializePeerConnection();
		}
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::SetRemoteSdp_s, this, sdp, type));
		//SetRemoteSdp_s(sdp, type);
	}
	void lylPeerConnection::SetRemoteSdp_s(const char* sdp, const char* type)
	{
		absl::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(type);
		webrtc::SdpParseError error;
		std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(*type_maybe, sdp, &error);
		if (!session_description)
		{
			RTC_LOG(WARNING) << "Can't parse received session description message.  SdpParseError was: " << error.description;
			return;
		}
		this->DealSdpOtherInfo(session_description.get());
		peer_connection_->SetRemoteDescription(SetSessionDescriptionObserverProxy::Create(), session_description.release());

		if (type_maybe == webrtc::SdpType::kOffer)
		{
			peer_connection_->CreateAnswer(this, sdpOptions_);
		}
		else  if (type_maybe == webrtc::SdpType::kAnswer)
		{
		}
		this->isCompletedSdpChange = true;
	}

	void lylPeerConnection::AddRemoteIceCandidate(char* candidate, char* sdp_mid, int sdp_mline_index)
	{
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::AddRemoteIceCandidate_s, this, candidate, sdp_mid, sdp_mline_index));
		this->DealCacheCandidates_s();
		//AddRemoteIceCandidate_s(candidate, sdp_mid, sdp_mline_index);
	}
	void lylPeerConnection::AddRemoteIceCandidate_s(char* candidate, char* sdp_mid, int sdp_mline_index)
	{
		webrtc::SdpParseError error;
		auto candidateClass = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &error);
		if (!candidateClass)
		{
			RTC_LOG(WARNING) << "Can't parse received candidate message.  SdpParseError was: " << error.description;
			return;
		}
		cacheCandidates_.push_back(candidateClass);
	}

	void lylPeerConnection::DealCacheCandidates()
	{
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::DealCacheCandidates_s, this));
		//DealCacheCandidates_s();
	}
	void lylPeerConnection::DealCacheCandidates_s()
	{
		if (this->isCompletedSdpChange == false)
		{
			return;
		}
		RTC_LOG(WARNING) << "deal candidate cache.candidate count: " << cacheCandidates_.size();
		for (size_t i = 0; i < cacheCandidates_.size(); i++)
		{
			peer_connection_->AddIceCandidate(cacheCandidates_[i]);
		}
		cacheCandidates_.clear();
	}
	//***************************************PeerConnectionClientObserver implementation. 
	void lylPeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnSignalingChange_s, this, new_state));
	}
	void lylPeerConnection::OnSignalingChange_s(webrtc::PeerConnectionInterface::SignalingState new_state)
	{
		if (this->onSignalingStateHandle)
		{
			this->onSignalingStateHandle(new_state);
		}
	}

	void lylPeerConnection::OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnAddStream_s, this, stream));
	}
	void lylPeerConnection::OnAddStream_s(rtc::scoped_refptr<MediaStreamInterface> stream)
	{
		this->remoteStreams_.push_back(stream);
		if (this->onAddStreamHandle)
		{
			this->onAddStreamHandle((char*)stream->id().data());
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********VideoTracksCount:" << stream->GetVideoTracks().size();
		}
	}

	void lylPeerConnection::OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnRemoveStream_s, this, stream));
	}
	void lylPeerConnection::OnRemoveStream_s(rtc::scoped_refptr<MediaStreamInterface> stream)
	{
		if (this->onRemoveStreamHandle)
		{
			this->onRemoveStreamHandle((char*)stream->id().data());
		}
	}

	void lylPeerConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnAddTrack_s, this, receiver, streams));
	}
	void lylPeerConnection::OnAddTrack_s(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
	{
		auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(receiver->track().get());
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********trackType:" << track->kind();
		if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
			auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
			auto newVideoRender = new lylwebrtc::lylVideoRender(track->id().data(), receiver->streams()[0]->id().data(), video_track);
			video_track->AddOrUpdateSink(newVideoRender, rtc::VideoSinkWants());
			remoteVideoRenders.push_back(newVideoRender);
		}
		else if (track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
			//auto* audio_track = static_cast<webrtc::AudioTrackInterface*>(track);
		}

		if (this->onAddTrackHandle)
		{
			this->onAddTrackHandle((char*)receiver->streams()[0]->id().data(), (char*)track->id().data(), (char*)track->kind().data());
		}
	}

	void lylPeerConnection::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnRemoveTrack_s, this, receiver));
	}
	void lylPeerConnection::OnRemoveTrack_s(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
	{
		auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(receiver->track().release());
		if (track->kind() != webrtc::MediaStreamTrackInterface::kVideoKind) {
			return;
		}
		auto render = this->findVideoTrack(track->id());
		if (render)
		{
			//
			vector< lylwebrtc::lylVideoRender*>::iterator itr = std::find(this->remoteVideoRenders.begin(), this->remoteVideoRenders.end(), render);
			this->remoteVideoRenders.erase(itr);
			delete render;
		}

		if (this->onRemoveTrackHandle)
		{
			this->onRemoveTrackHandle((char*)receiver->streams()[0]->id().data(), (char*)track->id().data());
		}
	}

	void lylPeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnDataChannel_s, this, channel));
	}
	void lylPeerConnection::OnDataChannel_s(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
	{
		lylDataChannel* channelObserver = new lylDataChannel(channel->label().data());

		channelObserver->RegisterDataChannel(channel.get());

		dataChannels.push_back(channelObserver);

		if (this->onDataChannelHandle)
		{
			this->onDataChannelHandle((char*)channel->label().data());
		}
	}

	void lylPeerConnection::OnRenegotiationNeeded()
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnRenegotiationNeeded_s, this));
	}
	void lylPeerConnection::OnRenegotiationNeeded_s()
	{
	}

	void lylPeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnIceConnectionChange_s, this, new_state));
	}
	void lylPeerConnection::OnIceConnectionChange_s(webrtc::PeerConnectionInterface::IceConnectionState new_state)
	{
		if (this->onIceConnectionStateHandle)
		{
			this->onIceConnectionStateHandle(new_state);
		}
	}

	void lylPeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnIceGatheringChange_s, this, new_state));
	}
	void lylPeerConnection::OnIceGatheringChange_s(webrtc::PeerConnectionInterface::IceGatheringState new_state)
	{
		if (this->onIceGatheringStateHandle)
		{
			this->onIceGatheringStateHandle(new_state);
		}
	}

	void lylPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnIceCandidate_s, this, candidate));
	}
	void lylPeerConnection::OnIceCandidate_s(const webrtc::IceCandidateInterface* candidate)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********OnIceCandidate:" << candidate->candidate().ToString();
		if (this->onIceCandidateHandle)
		{
			std::string icesdp;
			if (!candidate->ToString(&icesdp))
			{
				return;
			}
			this->onIceCandidateHandle((char*)icesdp.data(), (char*)candidate->sdp_mid().data(), candidate->sdp_mline_index());
		}
	}
	//***************************************CreateSessionDescriptionObserver implementation.
	void lylPeerConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc)
	{
		RTC_LOG(LS_INFO) << "create sdp success" << ",type:";
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnSuccess_s, this, desc));
	}
	void lylPeerConnection::OnSuccess_s(webrtc::SessionDescriptionInterface* desc)
	{
		this->DealSdpOtherInfo(desc);
		peer_connection_->SetLocalDescription(SetSessionDescriptionObserverProxy::Create(), desc);
		std::string sdp;
		desc->ToString(&sdp);
		RTC_LOG(LS_INFO) << "create sdp success" << ",type:" << webrtc::SdpTypeToString(desc->GetType()) << ",***********" << sdp.data();
		if (this->onCreateSdpSuccess)
		{
			this->onCreateSdpSuccess(sdp.data(), webrtc::SdpTypeToString(desc->GetType()));
		}
	}

	void lylPeerConnection::OnFailure(webrtc::RTCError error)
	{
		Thread::Current()->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::OnFailure_s, this, &error));
	}
	void lylPeerConnection::OnFailure_s(webrtc::RTCError* error)
	{
		RTC_LOG(LS_INFO) << "create sdp success" << ToString(error->type()) << ": ***********" << error->message();
		if (this->onCreateSdpFailure)
		{
			this->onCreateSdpFailure(error->message());
		}
	}
	//***************************************methods 
	lylVideoRender* lylPeerConnection::findVideoTrack(std::string trackId)
	{
		/*	RTC_LOG(LS_INFO) << __FUNCTION__ << ",current search trackId:" << trackId;
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",current render size:" << this->remoteVideoRenders.size();*/

		for (size_t i = 0; i < this->remoteVideoRenders.size(); i++) {
			//RTC_LOG(LS_INFO) << __FUNCTION__ << ",loop search trackId:" << this->remoteVideoRenders[i]->trackId << "----streamLabel:" << this->remoteVideoRenders[i]->streamLabel;
			if (strcmp(this->remoteVideoRenders[i]->trackId.data(), trackId.data()) == 0) {
				return this->remoteVideoRenders[i];
			}
		}
		return NULL;
	}
	lylDataChannel* lylPeerConnection::findDataChannel(char* channelLabel)
	{
		for (size_t i = 0; i < this->dataChannels.size(); i++) {
			//RTC_LOG(LS_INFO) << __FUNCTION__ << ",loop search channelLabel:" << channelLabel << ",,,loop:" << this->dataChannels[i]->label;
			if (strcmp(this->dataChannels[i]->label.data(), channelLabel) == 0) {
				return this->dataChannels[i];
			}
		}
		return NULL;
	}

	bool lylPeerConnection::removeDataChannel(char* channelLabel)
	{
		return signaling_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::removeDataChannel_s, this, channelLabel));
	}
	bool lylPeerConnection::removeDataChannel_s(char* channelLabel)
	{
		auto channel = this->findDataChannel(channelLabel);
		if (channel)
		{
			vector<lylDataChannel*>::iterator itr = std::find(this->dataChannels.begin(), this->dataChannels.end(), channel);
			channel->Close();
			this->dataChannels.erase(itr);
			delete channel;
		}
		return true;
	}

	void lylPeerConnection::RemoveStream(char* streamId)
	{
		signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&lylPeerConnection::RemoveStream_s, this, streamId));
	}
	void lylPeerConnection::RemoveStream_s(char* streamId)
	{
		for (size_t i = 0; i < peer_connection_->remote_streams()->count(); i++)
		{
			auto mstream = peer_connection_->remote_streams()->at(i);
			if (strcmp(mstream->id().data(), streamId) == 0)
			{
				RTC_LOG(LS_INFO) << "find remote stream to delete,id" << streamId;
				for (const auto& vtrack : mstream->GetAudioTracks())
				{
					mstream->RemoveTrack(vtrack);
					vtrack->Release();
				}

				for (const auto& vtrack : mstream->GetVideoTracks())
				{
					mstream->RemoveTrack(vtrack);
					vtrack->Release();
				}
				peer_connection_->RemoveStream(mstream);
				mstream->Release();
			}
		}

		for (size_t i = 0; i < peer_connection_->local_streams()->count(); i++)
		{
			auto mstream = peer_connection_->local_streams()->at(i);
			if (strcmp(mstream->id().data(), streamId) == 0)
			{
				RTC_LOG(LS_INFO) << "find local stream to delete,id" << streamId;
				for (const auto& vtrack : mstream->GetAudioTracks())
				{
					mstream->RemoveTrack(vtrack);
					vtrack->Release();
				}

				for (const auto& vtrack : mstream->GetVideoTracks())
				{
					mstream->RemoveTrack(vtrack);
					vtrack->Release();
				}
				mstream->Release();
				peer_connection_->RemoveStream(mstream);
			}
		}
	}
	//********************************************************************************************
	lylPeerConnection::~lylPeerConnection()
	{
		this->DeletePeerConnection();
	}

}
