#ifndef lylMediaConstraints_H
#define lylMediaConstraints_H

#include "api/mediaconstraintsinterface.h"
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lylwebrtc {
	using namespace std;
	using namespace rtc;
	using namespace webrtc;

	class lylMediaConstraints :public rtc::RefCountInterface, public webrtc::MediaConstraintsInterface {
	public:
		lylMediaConstraints() {}
	protected:
		virtual ~lylMediaConstraints() {}

	public:
		/** Constraint keys for media sources. */
		static const char* kMinAspectRatio;
		static const char* kMaxAspectRatio;
		static const char* kMaxWidth;
		static const char* kMinWidth;
		static const char* kMaxHeight;
		static const char* kMinHeight;
		static const char* kMaxFrameRate;
		static const char* kMinFrameRate;
		/** The value for this key should be a base64 encoded string containing
		 *  the data from the serialized configuration proto.
		 */
		static const char* kAudioNetworkAdaptorConfig;

		/** Constraint keys for generating offers and answers. */
		static const char* kIceRestart;
		static const char* kOfferToReceiveAudio;
		static const char* kOfferToReceiveVideo;
		static const char* kVoiceActivityDetection;
		/** Constraint values for Boolean parameters. */
		static const char* kValueTrue;
		static const char* kValueFalse;
		/**  . */
		static const char*  kCpuOveruseDetection;

	public:
		static rtc::scoped_refptr<lylMediaConstraints> Create();

		const webrtc::MediaConstraintsInterface::Constraints& GetMandatory() const override {
			return mandatory_;
		}

		const webrtc::MediaConstraintsInterface::Constraints& GetOptional() const override {
			return optional_;
		}

		void AddMandatoryConstraint(const char* key, const char* value);

		void AddOptionalConstraint(const char* key, const char* value);

	private:
		webrtc::MediaConstraintsInterface::Constraints mandatory_;
		webrtc::MediaConstraintsInterface::Constraints optional_;
	};

};  // namespace libwebrtc

#endif  //  
