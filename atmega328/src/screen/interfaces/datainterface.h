#ifndef DATAINTERFACE_H_
#define DATAINTERFACE_H_

#include <stdint.h>

class DataInterface
{
public:
	virtual ~DataInterface() {}

	virtual void send_data_byte(const uint8_t byte) = 0;

	virtual uint8_t get_data_byte() = 0;
};

#endif
