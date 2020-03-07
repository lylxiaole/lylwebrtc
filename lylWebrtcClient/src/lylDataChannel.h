#ifndef LIB_WEBRTC_RTC_DATA_CHANNEL_IMPL_HXX
#define LIB_WEBRTC_RTC_DATA_CHANNEL_IMPL_HXX

#include "globalHraders.h"
#include "api/datachannelinterface.h"
#include "rtc_base/criticalsection.h"

namespace lylwebrtc {

	class lylDataChannel : public webrtc::DataChannelObserver {
	public:
		_onChannelStateChanged onStateChangeHandle = NULL;
		_onChannelMessage onMessageHandle = NULL;

		lylDataChannel(const char* label_);

		void RegisterDataChannel(webrtc::DataChannelInterface* rtc_data_channel);

		void Send(const char* data, int length, bool binary = false);
		void Close();
		std::string label;
		int id() const;
		webrtc::DataChannelInterface::DataState state();

		rtc::scoped_refptr<webrtc::DataChannelInterface> rtc_data_channel()
		{
			return rtc_data_channel_;
		}

	protected:
		virtual void OnStateChange() override;
		virtual void OnMessage(const webrtc::DataBuffer& buffer) override;

	private:
		rtc::scoped_refptr<webrtc::DataChannelInterface> rtc_data_channel_;
		std::unique_ptr<rtc::CriticalSection> crit_sect_;
		webrtc::DataChannelInterface::DataState state_;
	};

};  // namespace libwebrtc

#endif  // !LIB_WEBRTC_RTC_DATA_CHANNEL_IMPL_HXX
