#include "Packet.h"
#include <Windows.h>

Packet::Packet()
{

}

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

int Packet::getSize()
{
	return this->size;
}

char* Packet::getBuffer()
{
	return this->buffer;
}