
#include "p2ptestMain.h"
#include "conductor.h"
//
 
//
#include "rtc_base/checks.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/win32socketinit.h"
#include "rtc_base/win32socketserver.h"


rtc::scoped_refptr<Conductor> caller = nullptr;
rtc::scoped_refptr<Conductor> callee = nullptr;


void onCallerSendMessage(std::string* json_object)
{ 
	callee->OnMessageFromPeer(1, *json_object);
	//要等待双方都设置了远程SDP，才能交换icecandidate
	if (callee->isCompletedSdpChange == true && caller->isCompletedSdpChange == true)
	{
		callee->DealCacheCandidates();
		caller->DealCacheCandidates();
	}
}

void onCalleeSendMessage(std::string* json_object)
{
	caller->OnMessageFromPeer(2, *json_object);
	//要等待双方都设置了远程SDP，才能交换icecandidate
	if (callee->isCompletedSdpChange == true && caller->isCompletedSdpChange == true)
	{
		callee->DealCacheCandidates();
		caller->DealCacheCandidates();
	}
}


int main22(int argc, char* argv[])
{ 
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	rtc::InitializeSSL();
	 
	caller = rtc::scoped_refptr<Conductor>(new rtc::RefCountedObject<Conductor>());
	caller->OnSendMessage = &onCallerSendMessage;
	//
	callee = rtc::scoped_refptr<Conductor>(new rtc::RefCountedObject<Conductor>());
	callee->OnSendMessage = &onCalleeSendMessage;
	//
	caller->ConnectToPeer(2);


	int i = 0;
	std::cin >> i;
	return 1;
}
 