#ifndef PACKETSTRUCTS_H
#define PACKETSTRUCTS_H

#pragma once
#include "PacketType.h"
#include "Packet.h"
#include "FileTransferData.h"
#include <string>

namespace PS // Packet structures namespace
{
	struct ChatMessage
	{

	public:
		ChatMessage(std::string);
		Packet toPacket();		 // Converts ChatMessage to packet

	private:
		std::string message;	 // Message held in ChatMessage 
	
	};


	struct FileDataBuffer
	{
	public:
		Packet toPacket();							    // Converts FileDataBuffer -> Packet
		char databuffer[FileTransferData::buffersize];	// Buffer of data to be sent
		int size;										// Size of data buffer
	};


}

#endif