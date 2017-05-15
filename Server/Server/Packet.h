#ifndef PACKET_H
#define PACKET_H

#pragma once

class Packet
{
public:

	// Constructors
	Packet();
	Packet(char* buffer, int size); // Will use existing allocated buffer and create packet from it
	Packet(const Packet& p);		// Will allocate new buffer, but copy buffer from packet argument

	int getSize();
	char* getBuffer();

private:
	int size;
	char* buffer;
};

#endif