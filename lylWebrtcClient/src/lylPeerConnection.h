#ifndef lylPeerConnection_Hxx
#define lylPeerConnection_Hxx

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "rtc_base/win32socketinit.h"

#include "lylVideoRender.h"
#include "lylDataChannel.h"
namespace lylwebrtc
{
	using namespace std;
	using namespace rtc;
	using namespace webrtc;
	class RTCStatsObtainer : public RTCStatsCollectorCallback {
	public:
		static rtc::scoped_refptr<RTCStatsObtainer> Create(rtc::scoped_refptr<const RTCStatsReport>* report_ptr = nullptr) {
			return rtc::scoped_refptr<RTCStatsObtainer>( new rtc::RefCountedObject<RTCStatsObtainer>(report_ptr));
		}

		void OnStatsDelivered(const rtc::scoped_refptr<const RTCStatsReport>& report) override 
		{ 
			report_ = report;
			if (report_ptr_)
				*report_ptr_ = report_;
		}

		rtc::scoped_refptr<const RTCStatsReport> report() const { 
			return report_;
		}

	protected:
		explicit RTCStatsObtainer( rtc::scoped_refptr<const RTCStatsReport>* report_ptr): report_ptr_(report_ptr) {}

	private: 
		rtc::scoped_refptr<const RTCStatsReport> report_;
		rtc::scoped_refptr<const RTCStatsReport>* report_ptr_;
	};

	class lylPeerConnection : public webrtc::CreateSessionDescriptionObserver, public webrtc::PeerConnectionObserver
	{

	private:
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

		rtc::scoped_refptr<MediaStreamInterface> localStream_;
		vector<rtc::scoped_refptr<MediaStreamInterface>> remoteStreams_;

		vector<lylDataChannel*> dataChannels;
		vector<lylVideoRender*> remoteVideoRenders;
		std::vector<webrtc::IceCandidateInterface*> cacheCandidates_;
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions sdpOptions_;

		Thread* worker_thread_;
		Thread* signaling_thread_;
		Thread* network_thread_;

		bool InitializePeerConnection();
		bool CreatePeerConnection(bool dtls);

		void DealSdpOtherInfo(webrtc::SessionDescriptionInterface * desc);
		std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice();
		rtc::scoped_refptr<webrtc::VideoTrackInterface> CreateVideoTrack();
		rtc::scoped_refptr<webrtc::VideoTrackInterface> CreateDesktopTrack();
		rtc::scoped_refptr<webrtc::AudioTrackInterface> CreateAudioTrack();

	public:
		_onSignalingState onSignalingStateHandle = NULL;
		_onIceGatheringState onIceGatheringStateHandle = NULL;
		_onRemoveStream onRemoveStreamHandle = NULL;
		_onRemoveTrack onRemoveTrackHandle = NULL;
		_onDataChannel onDataChannelHandle = NULL;
		_onAddTrack onAddTrackHandle = NULL;
		_onAddStream onAddStreamHandle = NULL;
		_onIceConnectionState onIceConnectionStateHandle = NULL;
		_onIceCandidate onIceCandidateHandle = NULL;
		_sdpCreateSuccess onCreateSdpSuccess = NULL;
		_sdpCreateFailure onCreateSdpFailure = NULL;
	public:
		lylPeerConnection(Thread* worker_thread, Thread* signaling_thread, Thread* network_thread);
		~lylPeerConnection() override;

		bool isCompletedSdpChange = false;
		webrtc::PeerConnectionInterface::RTCConfiguration config;

		_onFrame onLocalFrameHandle = nullptr;
		lylwebrtc::lylVideoRender* localVideoRender_ = nullptr;

		bool isVedio = false;
		uint32_t vedioIndex = 0;
		bool isAudio = true;
		uint32_t recordAudioIndex = 0;
		bool isDesktop = true;

		void CreateOffer();
		void SetRemoteSdp(const char * sdp, const char * type);
		void AddRemoteIceCandidate(char * candidate, char * sdp_mid, int sdp_mline_index);
		void DealCacheCandidates();
		void DeletePeerConnection();
		lylVideoRender* findVideoTrack(std::string trackId);
		lylDataChannel* findDataChannel(char* channelLabel);
		bool removeDataChannel(char* channelLabel);
		void RemoveStream(char * streamId);
		void CreateDataChannel(char * label);
		 
		const RTCStatsReport* GetPeerStats(); 
	private:
		void CreateOffer_s();
		void CreateDataChannel_s(char* label);
		void DealCacheDataChannels();
		void SetRemoteSdp_s(const char * sdp, const char * type);
		void AddRemoteIceCandidate_s(char * candidate, char * sdp_mid, int sdp_mline_index);
		void DealCacheCandidates_s();
		void OnSignalingChange_s(webrtc::PeerConnectionInterface::SignalingState new_state);
		void OnAddStream_s(rtc::scoped_refptr<MediaStreamInterface> stream);
		void OnRemoveStream_s(rtc::scoped_refptr<MediaStreamInterface> stream);
		void OnAddTrack_s(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams);
		void OnRemoveTrack_s(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver);
		void OnDataChannel_s(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);
		void OnRenegotiationNeeded_s();
		void OnIceConnectionChange_s(webrtc::PeerConnectionInterface::IceConnectionState new_state);
		void OnIceGatheringChange_s(webrtc::PeerConnectionInterface::IceGatheringState new_state);
		void OnIceCandidate_s(const webrtc::IceCandidateInterface * candidate);
		void OnSuccess_s(webrtc::SessionDescriptionInterface * desc);
		void OnFailure_s(webrtc::RTCError* error);
		void AddTracks();

		bool removeDataChannel_s(char * channelLabel);
		void RemoveStream_s(char * streamId);
		void DeletePeerConnection_a();
		void DeletePeerConnection_b();

	protected:
		// PeerConnectionClientObserver implementation. 
		void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
		void OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream) override;
		void OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream)  override;
		void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&streams) override;
		void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
		void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
		void OnRenegotiationNeeded() override;
		void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
		void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
		void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
		// CreateSessionDescriptionObserver implementation.
		void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
		void OnFailure(webrtc::RTCError error) override;
		// 
	};

	class SetSessionDescriptionObserverProxy : public webrtc::SetSessionDescriptionObserver {
	public:
		static SetSessionDescriptionObserverProxy* Create()
		{
			return new rtc::RefCountedObject<SetSessionDescriptionObserverProxy>();
		}
		void OnSuccess()  override {
			RTC_LOG(INFO) << __FUNCTION__;

		}
		void OnFailure(const std::string& error) override {
			RTC_LOG(INFO) << __FUNCTION__ << " " << error;

		}

	protected:
		SetSessionDescriptionObserverProxy() {}
		~SetSessionDescriptionObserverProxy() {}
	};



};
#endif