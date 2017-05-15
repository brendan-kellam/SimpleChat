#include "PacketManger.h"

bool PacketManager::HasPendingPackets()
{
	return (queue_packets.size() > 0); 
}


void PacketManager::Append(Packet p)
{
	std::lock_guard<std::mutex> lock(mutex_packets); // lock/unlock mutex handler
	queue_packets.push(p); // Add packet to queue
}

Packet PacketManager::Retrieve()
{
	std::lock_guard<std::mutex> lock(mutex_packets); // lock/unlock mutex handler
	Packet p = queue_packets.front(); // Get packet from front of queue
	queue_packets.pop();			  // Remove packet from queue
	return p;
}

