#include "Packet.h"
#include <Windows.h>

Packet::Packet()
{ }

Packet::Packet(char* buffer, int size)
{
	this->buffer = buffer;
	this->size = size;
}

// Allocate new block for buffer
Packet::Packet(const Packet& p)
{
	size = p.size;
	buffer = new char[size];
	memcpy(this->buffer, p.buffer, size); // Copy buffer from p to this
}

Packet::Packet(PacketType p)
{
	buffer = new char[sizeof(int32_t)];			  // Create buffer to store packet type data 
	int32_t packettype = (int32_t) p;			  // store packet type in 32 bit integer
	packettype = htonl(packettype);				  
	memcpy(buffer, &packettype, sizeof(int32_t)); // Copy from 32 bit integer to buffer
	size = sizeof(int32_t);						  // Set size to size of 32 bit integer
}

int Packet::getSize()
{
	return this->size;
}

char* Packet::getBuffer()
{
	return this->buffer;
}