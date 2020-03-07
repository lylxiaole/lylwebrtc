
#include "lylDesktopCapturer.h"
#include "modules/video_coding/utility/quality_scaler.h"
namespace lylwebrtc {

	lylDesktopCapturer::lylDesktopCapturer(std::unique_ptr<webrtc::DesktopCapturer> desktopcapturer) :capturer(std::move(desktopcapturer))
	{
		std::vector<cricket::VideoFormat> formats;
		formats.push_back(cricket::VideoFormat(800, 600, cricket::VideoFormat::FpsToInterval(20), cricket::FOURCC_ARGB));
		SetSupportedFormats(formats);
	}

	lylDesktopCapturer ::~lylDesktopCapturer() {}

	cricket::CaptureState lylDesktopCapturer::Start(const cricket::VideoFormat& capture_format)
	{ 
		/************/
		cricket::VideoFormat supported;
		if (GetBestCaptureFormat(capture_format, &supported))
		{
			SetCaptureFormat(&supported);
		}

		SetCaptureState(cricket::CS_RUNNING);
		capturer->Start(this);
		CaptureFrame();
		return cricket::CS_RUNNING;
	}

	void lylDesktopCapturer::Stop() {
		SetCaptureState(cricket::CS_STOPPED);
		SetCaptureFormat(NULL);
	}

	bool lylDesktopCapturer::IsRunning() {
		return capture_state() == cricket::CS_RUNNING;
	}

	bool lylDesktopCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
	{
		fourccs->push_back(cricket::FOURCC_I420);
		fourccs->push_back(cricket::FOURCC_MJPG);
		fourccs->push_back(cricket::FOURCC_ARGB);
		return true;
	}

	void lylDesktopCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) {
		if (result != webrtc::DesktopCapturer::Result::SUCCESS)
		{
			return;
		}
		 
		int width = frame->size().width();
		int height = frame->size().height(); 

		if (!i420_buffer_ || !i420_buffer_.get())
		{
			i420_buffer_ = webrtc::I420Buffer::Create(width, height);
		}
		else if (i420_buffer_ && i420_buffer_->width() * i420_buffer_->height() < width * height)
		{
			i420_buffer_ = nullptr;
			i420_buffer_ = webrtc::I420Buffer::Create(width, height);
		}
		  
		libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
			i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
			i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
			i420_buffer_->StrideV(), 0, 0, width, height, width,
			height, libyuv::kRotate0, libyuv::FOURCC_ARGB);
		  
		OnFrame(webrtc::VideoFrame(i420_buffer_, 0, 0, webrtc::kVideoRotation_0), width, height); 
	}


	//void lylDesktopCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) {
	//

	//	if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
	//		return;
	//	}
	//	int width = frame->rect().width();
	//	int height = frame->rect().height();
	//	rtc::scoped_refptr<webrtc::I420Buffer> I420buffer = webrtc::I420Buffer::Create(width, height);

	//	const int conversionResult = libyuv::ConvertToI420(frame->data(), frame->stride()*webrtc::DesktopFrame::kBytesPerPixel,
	//		I420buffer->MutableDataY(), I420buffer->StrideY(),
	//		I420buffer->MutableDataU(), I420buffer->StrideU(),
	//		I420buffer->MutableDataV(), I420buffer->StrideV(),
	//		0, 0,
	//		width, height,
	//		width, height,
	//		libyuv::kRotate0, ::libyuv::FOURCC_ARGB);

	//	if (conversionResult < 0)
	//	{
	//		return;
	//	}

	//	int m_height = (int)(height * 1);//目标高度
	//	int m_width = (int)(width *  1);//目标宽度
	//	height = m_height;
	//	width = m_width;

	//	webrtc::VideoFrame videoFrame(I420buffer, webrtc::VideoRotation::kVideoRotation_0, rtc::TimeMicros());

	//	int stride_y = width;
	//	int stride_uv = (width + 1) / 2;
	//	rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer = webrtc::I420Buffer::Create(width, height, stride_y, stride_uv, stride_uv);
	//	scaled_buffer->ScaleFrom(*videoFrame.video_frame_buffer()->ToI420());
	//	webrtc::VideoFrame resframe = webrtc::VideoFrame(scaled_buffer, webrtc::kVideoRotation_0, rtc::TimeMicros());
	//	OnFrame(resframe, m_width, m_height);
	//	/*资源释放*/
	//	I420buffer = nullptr;
	//	scaled_buffer = nullptr;
	//}




	void lylDesktopCapturer::OnMessage(rtc::Message* msg)
	{
		if (msg->message_id == 0)
		{
			CaptureFrame();
		}
	}

	void lylDesktopCapturer::CaptureFrame()
	{ 
		capturer->CaptureFrame();
		rtc::Location loc;
		if (rtc::Thread::Current())
		{
			rtc::Thread::Current()->PostDelayed(loc, 95, this, 0);
		}

	}
}