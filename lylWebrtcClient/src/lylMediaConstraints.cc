#include "lylMediaConstraints.h"

namespace lylwebrtc {

	scoped_refptr<lylMediaConstraints> lylMediaConstraints::Create()
	{
		scoped_refptr<lylMediaConstraints> constraints = scoped_refptr<lylMediaConstraints>(new RefCountedObject<lylMediaConstraints>());
		return constraints;
	}

	const char* lylMediaConstraints::kMinAspectRatio = webrtc::MediaConstraintsInterface::kMinAspectRatio;
	const char* lylMediaConstraints::kMaxAspectRatio = webrtc::MediaConstraintsInterface::kMaxAspectRatio;
	const char* lylMediaConstraints::kMinWidth = webrtc::MediaConstraintsInterface::kMinWidth;
	const char* lylMediaConstraints::kMaxWidth = webrtc::MediaConstraintsInterface::kMaxWidth;
	const char* lylMediaConstraints::kMinHeight = webrtc::MediaConstraintsInterface::kMinHeight;
	const char* lylMediaConstraints::kMaxHeight = webrtc::MediaConstraintsInterface::kMaxHeight;
	const char* lylMediaConstraints::kMinFrameRate = webrtc::MediaConstraintsInterface::kMinFrameRate;
	const char* lylMediaConstraints::kMaxFrameRate = webrtc::MediaConstraintsInterface::kMaxFrameRate;
	const char* lylMediaConstraints::kAudioNetworkAdaptorConfig = webrtc::MediaConstraintsInterface::kAudioNetworkAdaptorConfig;
	const char* lylMediaConstraints::kIceRestart = webrtc::MediaConstraintsInterface::kIceRestart;
	const char* lylMediaConstraints::kOfferToReceiveAudio = webrtc::MediaConstraintsInterface::kOfferToReceiveAudio;
	const char* lylMediaConstraints::kOfferToReceiveVideo = webrtc::MediaConstraintsInterface::kOfferToReceiveVideo;
	const char* lylMediaConstraints::kVoiceActivityDetection = webrtc::MediaConstraintsInterface::kVoiceActivityDetection;
	const char* lylMediaConstraints::kValueTrue = webrtc::MediaConstraintsInterface::kValueTrue;
	const char* lylMediaConstraints::kValueFalse = webrtc::MediaConstraintsInterface::kValueFalse;
	const char* lylMediaConstraints::kCpuOveruseDetection = webrtc::MediaConstraintsInterface::kCpuOveruseDetection;

	void lylMediaConstraints::AddMandatoryConstraint(const char* key, const char* value) {
		webrtc::MediaConstraintsInterface::Constraint constraint(key, value);
		mandatory_.push_back(constraint);
	}
	void lylMediaConstraints::AddOptionalConstraint(const char* key, const char* value) {
		webrtc::MediaConstraintsInterface::Constraint constraint(key, value);
		optional_.push_back(constraint);
	}
};  // namespace libwebrtc
