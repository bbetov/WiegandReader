#include "WiegandReader.h"

volatile unsigned int WiegandReader::currentBitCount = 0; // what is the current bit count
volatile uint64_t WiegandReader::bitHolder = 0; // holds the bits received
volatile unsigned long WiegandReader::lastBitReceived = 0; // the time since the last bit was received
volatile bool WiegandReader::IsInitialized = false;

void WiegandReader::Initialize()
{
	// Arduino Uno only has 2 pins for interrupts
	Reset();
	pinMode(Data0Pin, INPUT);
	pinMode(Data1Pin, INPUT);
	attachInterrupt(digitalPinToInterrupt(Data0Pin), Data0IntHandler, FALLING);	// high to low
	attachInterrupt(digitalPinToInterrupt(Data1Pin), Data1IntHandler, FALLING);	// high to low
	IsInitialized = true;
}

bool WiegandReader::IsAvailable()
{
	if (!IsInitialized)
		Initialize();
	if (currentBitCount >= 26)
	{
		unsigned long timeDiff = millis() - lastBitReceived;
		if (timeDiff > 25)
			return true;
	}

	return false;
}

bool WiegandReader::Read(RFIDInfo& rfid)
{
	bool retCode = false;
	noInterrupts();
	rfid.NumBits = 0;
	rfid.Data = 0;
	Serial.print("Bit Count: ");
	Serial.println(currentBitCount);

	if (currentBitCount >= 26)
	{
		rfid.NumBits = currentBitCount;
		rfid.Data = bitHolder;
		retCode = true;
	}

	Reset();
	interrupts();
	return retCode;
}

void WiegandReader::CheckTimingAndReset()
{
	unsigned long timeDiff = millis() - lastBitReceived;
	if (timeDiff > 25) // Must be a messed up protocol
	{
		Reset();
	}
}
void WiegandReader::Reset()
{
	currentBitCount = 0;
	bitHolder = 0;
	lastBitReceived = 0;
}

void WiegandReader::Data0IntHandler()
{
	CheckTimingAndReset();
	currentBitCount++;
	if (currentBitCount > 64)
		Reset();
	bitHolder = bitHolder << 1;
	lastBitReceived = millis();
}

void WiegandReader::Data1IntHandler()
{
	CheckTimingAndReset();
	currentBitCount++;
	if (currentBitCount > 64)
		Reset();
	bitHolder = bitHolder << 1;
	bitHolder |= 1;
	lastBitReceived = millis();
}