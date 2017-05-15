#ifndef PACKETSTRUCTS_H
#define PACKETSTRUCTS_H

#pragma once
#include "PacketType.h"
#include "Packet.h"
#include <string>

namespace PS // Packet structures namespce
{
	struct ChatMessage
	{
	public:
		ChatMessage(std::string);
		Packet toPacket();		 // Converts ChatMessage to packet

	private:
		std::string message;

	};

}

#endif