// WiegandReader.h

#ifndef _WIEGANDREADER_h
#define _WIEGANDREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct RFIDInfo
{
	unsigned long NumBits;
	uint64_t Data;
};

class WiegandReader
{
public:
	// The arduino data pins to attach the interrupts on
	static const int Data0Pin = 2;
	static const int Data1Pin = 3;
	
	// Returns true if the data on the DATA0/DATA1 has stopped
	// arriving for at least ~25ms and also the number of bits 
	// received is at least 26
	bool IsAvailable();

	// Once called the subsequent calls to Read will return false
	// unless a new data has been received
	bool Read(RFIDInfo& rfid);
private:
	// Must be called after construction
	void Initialize();

	static void Data0IntHandler();
	static void Data1IntHandler();
	static void CheckTimingAndReset();
	volatile static unsigned int currentBitCount; // what is the current bit count
	volatile static uint64_t bitHolder; // holds the bits received
	volatile static unsigned long lastBitReceived; // the time since the last bit was received
	static void Reset();
	volatile static bool IsInitialized;
};

#endif

