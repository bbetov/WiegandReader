#include "WiegandReader.h"

WiegandReader rfid;
// End code for DATA0 and DATA1
void setup()
{
	Serial.begin(57600);
	while (!Serial);

	delay(10);
}

void loop()
{
	if (rfid.IsAvailable())
	{
		RFIDInfo rInfo;
		uint32_t facility = 0, card = 0;
		if (rfid.Read(rInfo) && ParseCardValue(facility, card, rInfo))
		{
			Serial.print("ID: ");
			Serial.println(card);
			Serial.print("Facility: ");
			Serial.println(facility);
		}
	}
}

bool ParseWiegand26(uint32_t& facility, uint32_t& card, const uint64_t data)
{
	uint64_t bitHolder = data;
	uint64_t bitHolderParity = data;
	uint64_t one = 1;
	unsigned int oddParity = bitHolder & 0x1UL;
	bitHolder >>= 1; // Skip the parity
	card = bitHolder & 0xFFFFUL;
	bitHolder >>= 16; // Skip the card code
	facility = bitHolder & 0xFFUL;
	bitHolder >>= 8; // Skip the facility code
	unsigned int evenParity = bitHolder & 0x1UL;

#ifdef _DEBUG
	Serial.print("Even Parity: ");
	Serial.println(evenParity, BIN);
	Serial.print("Facility Code: ");
	Serial.println(facility, BIN);
	Serial.print("Card Code: ");
	Serial.println(card, BIN);
	Serial.print("Odd Parity: ");
	Serial.println(oddParity, BIN);
#endif // DEBUG


	// Check Even Parity
	unsigned int oddCount = 0;
	for (int cnt = 0; cnt < 13; cnt++)
	{
		oddCount += bitHolderParity & one;
		bitHolderParity >>= 1;
	}

	unsigned int evenCount = 0;
	for (int cnt = 0; cnt < 13; cnt++)
	{
		evenCount += bitHolderParity & one;
		bitHolderParity >>= 1;
	}

	if (evenCount % 2 != 0)
	{
#ifdef _DEBUG
		Serial.println("Even Parity Mismatch!");
#endif
		return false;
	}

	if (oddCount % 2 == 0)
	{
#ifdef _DEBUG
		Serial.println("Odd Parity Mismatch!");
#endif
		return false;
	}

	return true;
}

uint64_t GetMask(const int numBits)
{
	uint64_t one = 1;
	uint64_t retVal = 0;
	for (int i = 0; i < numBits; i++)
	{
		retVal |= one;
		one <<= 1;
	}
	return retVal;
}

bool ParseWiegand(uint32_t& facility, uint32_t& card, const uint64_t data, 
	const int cardBits, const int facilityBits)
{
	uint64_t bitHolder = data;
#ifdef _DEBUG
	unsigned int oddParity = bitHolder & 0x1UL;
#endif
	bitHolder >>= 1; // Kill the parity
	card = bitHolder & GetMask(cardBits);
	bitHolder >>= cardBits; // Skip the card code
	facility = bitHolder & GetMask(facilityBits);
	bitHolder >>= facilityBits; // Skip the facility code
#ifdef _DEBUG
	unsigned int evenParity = bitHolder & 0x1UL;
#endif

#ifdef _DEBUG
	Serial.print("Even Parity: ");
	Serial.println(evenParity, BIN);
	Serial.print("Facility Code: ");
	Serial.println(facility, BIN);
	Serial.print("Card Code: ");
	Serial.println(card, BIN);
	Serial.print("Odd Parity: ");
	Serial.println(oddParity, BIN);
#endif

	return true;
}

bool ParseCardValue(uint32_t& facility, uint32_t& card, const RFIDInfo& ri)
{
	card = facility = 0;
	switch (ri.NumBits)
	{
	case 26:
		return ParseWiegand26(facility, card, ri.Data);
	case 32:
	case 35:
	case 37:
		return ParseWiegand(facility, card, ri.Data, 20, ri.NumBits - 20 - 2);
	}

	return false;
}

