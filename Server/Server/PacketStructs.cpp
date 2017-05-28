#include "PacketStructs.h"
#include <Windows.h>

namespace PS
{
	ChatMessage::ChatMessage(std::string message)
	{
		this->message = message;
	}

	Packet ChatMessage::toPacket()
	{
		// Calculate total size of buffer for our chat message packet (int bytes)
		const int packetsize = sizeof(int32_t) * 2 + message.size()*sizeof(char);
		char* buffer = new char[packetsize];

		// Convert packettype and message size to network byte order
		int32_t packettype = htonl((int32_t)PacketType::ChatMessage);
		int32_t messagesize = htonl(message.size());

		// Copy packettype and message size to buffer
		memcpy(buffer, &packettype, sizeof(int32_t));
		memcpy(buffer + sizeof(int32_t), &messagesize, sizeof(int32_t));

		// Copy message to buffer
		memcpy(buffer + sizeof(int32_t) * 2, message.c_str(), message.size()*sizeof(char));

		Packet p(buffer, packetsize);

		return p;
	}

	Packet FileDataBuffer::toPacket()
	{
		const int packetsize = sizeof(int32_t) * 2 + size; // total size of packet
														   // | 4 bytes = Packet type  | 4 bytes = buffer size | size = buffer size |
		char* buffer = new char[packetsize];
		int32_t packettype = htonl((int32_t)PacketType::FileTransferByteBuffer);
		int32_t buffersize = htonl(size);

		// Move values to buffer
		memcpy(buffer, &packettype, sizeof(int32_t));
		memcpy(buffer + sizeof(int32_t), &buffersize, sizeof(int32_t));
		memcpy(buffer + sizeof(int32_t) * 2, databuffer, size);

		Packet p(buffer, packetsize);
		return p;
	}
}