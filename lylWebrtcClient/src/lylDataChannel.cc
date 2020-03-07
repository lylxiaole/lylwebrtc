#include "lylDataChannel.h"
#include "pc/datachannel.h"
namespace lylwebrtc {

	lylDataChannel::lylDataChannel(const char* label_) : label(label_), crit_sect_(new rtc::CriticalSection())
	{
	}
	void lylDataChannel::RegisterDataChannel(webrtc::DataChannelInterface* rtc_data_channel)
	{
		rtc_data_channel_ = rtc::scoped_refptr<webrtc::DataChannelInterface>(rtc_data_channel);
		rtc_data_channel_->RegisterObserver(this);
	}

	void lylDataChannel::Send(const char* data, int length, bool binary /*= false*/)
	{
		//auto channelcast = (webrtc::DataChannel*) rtc_data_channel_.get();

		if (binary)
		{
			rtc::CopyOnWriteBuffer binary(data);
			webrtc::DataBuffer buffer(binary, true);
			rtc_data_channel_->Send(buffer);
		}
		else
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << ",********** send data length:" << length << ",data:" << data;
			webrtc::DataBuffer buffer(data);
			rtc_data_channel_->Send(buffer);
		}
	}

	void lylDataChannel::Close()
	{
		rtc_data_channel_->UnregisterObserver();
		rtc_data_channel_->Close();
	}


	int lylDataChannel::id() const
	{
		return rtc_data_channel_->id();
	}

	void lylDataChannel::OnStateChange()
	{
		webrtc::DataChannelInterface::DataState state = rtc_data_channel_->state();
		state_ = state;
		rtc::CritScope(crit_sect_.get());
		if (this->onStateChangeHandle)
		{
			this->onStateChangeHandle(state_, (char*)this->label.data());
		}
	}

	webrtc::DataChannelInterface::DataState lylDataChannel::state()
	{
		return state_;
	}

	void lylDataChannel::OnMessage(const webrtc::DataBuffer& buffer)
	{
		if (this->onMessageHandle)
		{

			this->onMessageHandle(buffer.data.data<char>(), buffer.data.size(), buffer.binary, (char*)this->label.data());
		}
	}

};  // namespace libwebrtc
