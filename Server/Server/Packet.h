#ifndef PACKET_H
#define PACKET_H

#pragma once
#include "PacketType.h"
#include <stdint.h>

class Packet
{
public:

	// Constructors
	Packet();
	Packet(char* buffer, int size); // Will use existing allocated buffer and create packet from it
	Packet(const Packet& p);		// Will allocate new buffer, but copy buffer from packet argument
	Packet(PacketType p);			// used for when sending a packet that only containes a packet type

	int getSize();
	char* getBuffer();

private:
	int size;
	char* buffer;
};

#endif