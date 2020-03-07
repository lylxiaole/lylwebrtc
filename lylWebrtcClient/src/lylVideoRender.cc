
#include "lylVideoRender.h"
#include "third_party/libyuv/include/libyuv.h" 
#include "api/video/i420_buffer.h"

namespace lylwebrtc {

	lylVideoRender::lylVideoRender(const char* trackId_, const char* streamLabel_, rtc::scoped_refptr<webrtc::VideoTrackInterface> videotrack):videotrack_(videotrack.get())
	{
		this->trackId = std::string(trackId_);
		this->streamLabel = std::string(streamLabel_);
	 
	}

	lylVideoRender::~lylVideoRender()
	{

	}

	void lylVideoRender::OnFrame(const webrtc::VideoFrame& frame)
	{ 
		//RTC_LOG(LS_INFO) << __FUNCTION__ << ",**********OnFrame,trackId=" << trackId;
		if (this->onFrameHandle)
		{ 
			size_t buffer_size = (frame.width() * frame.height()) * (32 >> 3);
			uint8_t* dst_argb = new uint8_t[buffer_size];
			this->ConvertToRGBA(frame, dst_argb);
			this->onFrameHandle(dst_argb, buffer_size, frame.width(), frame.height(), this->trackId.data());
			delete[] dst_argb;
		}
	}

	void lylVideoRender::RegisterListener(_onFrame handle)
	{
		this->onFrameHandle = handle;
	}

	void lylVideoRender::ConvertToRGBA(const webrtc::VideoFrame& frame, uint8_t* out_imagebuffer)
	{
		int stride = frame.width() * 32 / 8;

		rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(frame.video_frame_buffer()->ToI420());
		if (frame.rotation() != webrtc::kVideoRotation_0)
		{
			buffer = webrtc::I420Buffer::Rotate(*buffer, frame.rotation());
		}
	 
		libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
			buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
			out_imagebuffer, stride, buffer->width(), buffer->height());
	}


	void lylVideoRender::RemoveSink()
	{
		this->videotrack_->RemoveSink(this);
	}
}