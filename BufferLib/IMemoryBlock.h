#pragma once

	class IMemoryBlock
	{
	public:
		virtual ~IMemoryBlock() {}
		virtual const char* getMemory() const = 0; // Do I want this?
		virtual size_t getLength() const = 0;
		virtual size_t copy(size_t sourceOffset, size_t sourceLength, char* pDestination) const= 0;
		virtual const char& operator[](size_t offset) const = 0;
	};
