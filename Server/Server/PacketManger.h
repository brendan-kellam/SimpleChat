#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#pragma once
#include "Packet.h"
#include <queue>
#include <mutex>

/*
* NOTE: Linked list are FAR quicker than vector's when appending and retreaving
* Speed comarison: 
*	(appending)
*	1. Linked list, 2. Queues, 3. Vectors
*   (retreaving)
*   1. Queues, 2. Linked list, 3. Vectors
*
* For efficiency sake, we shall use queues for storing our packets
*/

class PacketManager
{
private:
	std::queue<Packet> queue_packets;
	std::mutex mutex_packets;

public:
	void Clear();
	bool HasPendingPackets();
	void Append(Packet p);
	Packet Retrieve();

};

#endif