#pragma once

#include <memory>
#include <string>

class IMemoryBlock;


	class BufferFragment
	{
	public:
		BufferFragment(std::shared_ptr<IMemoryBlock> pMemoryBlock, size_t offset, size_t length);
		BufferFragment(BufferFragment&& source);
		BufferFragment(const BufferFragment& source) : BufferFragment{ source, 0, source.getLength() } {}
		BufferFragment(const BufferFragment& source, size_t offset, size_t length);
		BufferFragment operator=(const BufferFragment& source);
		BufferFragment operator=(BufferFragment&& source);
		virtual ~BufferFragment();

		size_t getLength() const;
		const char& operator[](size_t offset) const;
		size_t copy(size_t offset, size_t length, char* pDestination) const;
		std::string asString() const;
	private:
		size_t getInitialOffset(std::shared_ptr<IMemoryBlock> pMemoryBlock, size_t offset);
		size_t getInitialLength(std::shared_ptr<IMemoryBlock> pMemoryBlock, size_t offset, size_t length);

		std::shared_ptr<IMemoryBlock> _memoryBlock;
		size_t _offset;
		size_t _length;
	};



