#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#pragma once

#include <iostream>


// PacketType enum class
enum class PacketType
{
	ChatMessage,				  // Send simple chat message
	FileTransferRequestFile,	  // [C->S] Request a file to transfer
	FileTransfer_EndOfFile,       // [S->C] Sent for when file transfer is completed
	FileTransferByteBuffer,       // [S->C] Sent before sending byte buffer for file transfer
	FileTransferRequestNextBuffer // [C->S] Sent to request the next buffer for file

};

// Override << operator for PacketType
inline std::ostream& operator<<(std::ostream& os, const PacketType& packettype)
{
	os << (int32_t)packettype;
	return os;
}

#endif