#include "../Header/External.h"
#include "../../Socket/Messages/Header/Messages.h"

void Asteria::External::DetectionThread()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(8));

		if (Memcury::Scanner::FindStringRef("E:\\GitHub\\1hack\\1hack\\x64\\Release\\1hAck Fortnite.pdb").IsValid())
			break;
	}

	Asteria::Socket::Messages::Detection(Socket::Detections::Detection::ExternalDetection, _str("1hAck Detected!"));
}

void Asteria::External::Setup()
{
	std::thread(DetectionThread).detach();
}
