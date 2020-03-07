
#ifndef netVideoRender_H
#define netVideoRender_H

#include "globalHraders.h" 

#include "api/mediastreaminterface.h"
#include "api/video/video_frame.h"

namespace lylwebrtc {
	class lylVideoRender :public rtc::VideoSinkInterface<webrtc::VideoFrame>
	{
	public:
		std::string trackId ;
		std::string streamLabel; 
		_onFrame onFrameHandle = NULL;

		lylVideoRender(const char* trackId_, const char* streamLabel_, rtc::scoped_refptr<webrtc::VideoTrackInterface> videotrack_);
		~lylVideoRender() override;
		void OnFrame(const webrtc::VideoFrame& frame) override;
		void RegisterListener(_onFrame handle);
		void RemoveSink();
	private:
		void ConvertToRGBA(const webrtc::VideoFrame& frame, uint8_t* out_imagebuffer);
	
		rtc::scoped_refptr<webrtc::VideoTrackInterface> videotrack_;
	};
};
#endif


