#include "BufferFragment.h"

//using namespace BufferLib;

#include "IMemoryBlock.h"

BufferFragment::BufferFragment(std::shared_ptr<IMemoryBlock> pMemoryBlock, size_t offset, size_t length) :
	_memoryBlock{pMemoryBlock},
	_offset{ getInitialOffset(pMemoryBlock, offset) },
	_length{ getInitialLength(pMemoryBlock, offset, length) }
{
}

BufferFragment::BufferFragment(BufferFragment && source) :
	_memoryBlock{source._memoryBlock},
	_offset{source._offset},
	_length{source._length}
{
}


BufferFragment::BufferFragment(const BufferFragment & source, size_t offset, size_t length) :
	_memoryBlock{source._memoryBlock},
	_offset{ offset + source._offset },
	_length{ length }
{
	// Check offset is within the source fragment
	if (_offset > (source._offset + source._length))
	{
		_offset = source._offset;
		_length = 0;
	}
	else if ((_offset + _length) > (source._offset + source._length))
	{
		// Length extends beyond source fragment
		_length = source._offset + source._length - _offset;
	}
}

BufferFragment BufferFragment::operator=(const BufferFragment & source)
{
	if (this == &source)
	{
		return *this;
	}
	_memoryBlock = source._memoryBlock;
	_offset = source._offset;
	_length = source._length;

	return *this;
}

BufferFragment BufferFragment::operator=(BufferFragment && source)
{
	_memoryBlock.swap(source._memoryBlock);
	_offset = source._offset;
	_length = source._length;
	source._offset = source._length = 0;
	return *this;
}

BufferFragment::~BufferFragment()
{
}

size_t BufferFragment::getLength() const
{
	return _length;
}

const char & BufferFragment::operator[](size_t offset) const
{
	return (*_memoryBlock)[offset + _offset];
	// TODO: access out of range
}

size_t BufferFragment::copy(size_t offset, size_t length, char * pDestination) const
{
	auto srcOffset{ offset + _offset };
	auto maxLength{ _length - offset };
	auto copyLength{ length > maxLength ? maxLength : length };
	
	return _memoryBlock->copy(srcOffset, copyLength , pDestination);
}

std::string BufferFragment::asString() const
{
	std::string result("Fragment:\"");

	char* pTempStr = new char[getLength() + 1];
	pTempStr[getLength()] = 0;
	copy(0, getLength(), pTempStr);

	std::string contents(pTempStr);
	delete[] pTempStr;
	result += contents;

	result += "\"";
	return result;
}



size_t BufferFragment::getInitialOffset(std::shared_ptr<IMemoryBlock> xpMemoryBlock, size_t xOffset)
{
	size_t offset = xOffset;
	if (offset >= xpMemoryBlock->getLength())
	{
		offset = 0;
	}
	return offset;
}

size_t BufferFragment::getInitialLength(std::shared_ptr<IMemoryBlock> xpMemoryBlock, size_t xOffset, size_t xLength)
{
	auto memoryLength{ xpMemoryBlock->getLength() };
	auto length = xLength;
	if (xOffset >= memoryLength)
	{
		length = 0;
	}
	else if ((length + xOffset) > memoryLength)
	{
		length = memoryLength - xOffset;
	}
	return length;
}
