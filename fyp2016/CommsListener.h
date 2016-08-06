#pragma once


#include  "Packet.h"


class CommsListener {
public:
	virtual ~CommsListener() {}
	virtual void onEvent(Packet& packet) {};
};