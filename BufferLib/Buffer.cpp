#include "Buffer.h"
#include "BufferFragment.h"
#include "IMemoryBlock.h"

#include <sstream>
extern std::ostringstream gDebug;

Buffer::Buffer(std::shared_ptr<IMemoryBlock> pMemoryBlock)
{
	BufferFragment frag(pMemoryBlock, 0, pMemoryBlock->getLength());
	_fragments.push_back(frag);
}

Buffer::Buffer(const Buffer & srcBuffer, size_t offset)
	: Buffer(srcBuffer, offset, srcBuffer.getLength() - offset)
{
}

Buffer::Buffer(const Buffer & xSrcBuffer, size_t xOffset, size_t xLength)
{
	size_t offsetOfFragment = 0;
	auto offsetToCopyFrom = xOffset;
	auto lengthToDo = xLength;
	auto sourceLength = xSrcBuffer.getLength();
	if (offsetToCopyFrom >= sourceLength)
	{
		return;
	}
	if ((offsetToCopyFrom + xLength) > sourceLength)
	{
		lengthToDo = sourceLength - offsetToCopyFrom;
	}

	for (BufferFragment srcFrag : xSrcBuffer._fragments)
	{
		auto endOfFragment = offsetOfFragment + srcFrag.getLength();
		if (endOfFragment > offsetToCopyFrom) // We should copy this fragment
		{
			BufferFragment newFrag(srcFrag, offsetToCopyFrom - offsetOfFragment, lengthToDo);
			lengthToDo -= newFrag.getLength();
			_fragments.push_back(newFrag);
			offsetToCopyFrom += newFrag.getLength();
		}
		// else we still haven't found the start

		if (lengthToDo == 0)
		{
			break;
		}

		offsetOfFragment += srcFrag.getLength();
	}
}

Buffer & Buffer::operator+=(const Buffer & srcBuffer)
{
	// Copy to temp vector so it works if a buffer is appended to itself
	std::vector<BufferFragment> temp(srcBuffer._fragments);
	_fragments.reserve(_fragments.size() + temp.size());
	_fragments.insert(_fragments.end(), temp.begin(), temp.end());
	return *this;
}

size_t Buffer::getLength() const
{
	size_t length = 0;
	for (BufferFragment frag : _fragments)
	{
		length += frag.getLength();
	}
	return length;
}

const char & Buffer::operator[](size_t offset) const
{
	auto fragOffset = offset;
	for (auto frag : _fragments)
	{
		auto fragLength = frag.getLength();
		if (fragOffset < fragLength)
		{
			return frag[fragOffset];
		}
		fragOffset -= fragLength;
	}
	// TODO: Read past end of buffer
	return 0;
}

size_t Buffer::copy(size_t offset, size_t length, char * pDestination) const
{
	auto fragOffset = offset;
	auto bytesToWrite = length;
	auto pFragDest = pDestination;

	for (auto frag : _fragments)
	{
		auto fragLength = frag.getLength();
		if (fragOffset < fragLength)
		{
			auto fragBytesWritten = frag.copy(fragOffset, bytesToWrite, pFragDest);
			bytesToWrite -= fragBytesWritten;
			if (bytesToWrite == 0)
			{
				break;
			}
			pFragDest += fragBytesWritten;
			fragOffset = 0;
		}
		else
		{
			fragOffset -= fragLength;
		}
	}

	return length - bytesToWrite;
}


Buffer operator+(const Buffer& lhs, const Buffer& rhs)
{
	Buffer result(lhs);
	result += rhs;
	return result;
}

Buffer::const_itr Buffer::cbegin() const
{
	const_itr result(*this);
	return result;
}

Buffer::const_itr  Buffer::cend() const
{
	const_itr result(*this);
	// This seems a little inneficient.
	result += getLength();
	return result;
}


// Iterator definition

Buffer::const_itr::const_itr(const Buffer & xBuffer)
	: _buffer{ &xBuffer },
	_fragmentIterator{xBuffer._fragments.cbegin()},
	_fragmentOffset{0}
{}

Buffer::const_itr & Buffer::const_itr::operator++()
{
	if (*this != _buffer->cend())
	{
		++_fragmentOffset;
		if (_fragmentOffset >= _fragmentIterator->getLength())
		{
			++_fragmentIterator;
			_fragmentOffset = 0;
		}
		// else we're at the end.  Just don't increment
	}
	return *this;
}

Buffer::const_itr Buffer::const_itr::operator++(int)
{
	const_itr result{ *this };
	++(*this);
	return result;
}

Buffer::const_itr & Buffer::const_itr::operator--()
{
	if (_fragmentOffset > 0)
	{
		_fragmentOffset--;
	}
	else if (_fragmentIterator != _buffer->_fragments.cbegin())
	{
		--_fragmentIterator;
		_fragmentOffset = _fragmentIterator->getLength();
	}
	// else just don't decrement
	return *this;
}

Buffer::const_itr Buffer::const_itr::operator--(int)
{
	const_itr result(*this);
	--(*this);
	return result;
}

const char & Buffer::const_itr::operator*() const
{
	return (*_fragmentIterator)[_fragmentOffset];
}

const char & Buffer::const_itr::operator[](difference_type xIndex) const
{
	auto offset = _fragmentOffset;
	auto iter = _fragmentIterator;
	if (xIndex > 0)
	{
		auto toDo = xIndex;
		while ((toDo >= (iter->getLength() - offset)) &&
			(iter != _buffer->_fragments.cend()))
		{
			toDo -= (iter->getLength() - offset);
			++iter;
			offset = 0;
		}
		if (toDo >= (iter->getLength() - offset))
		{
			// read beyond bounds.  What to do?
		}
		offset = toDo;
	}
	else
	{
		auto toDo = -xIndex;
		while ((toDo > offset) &&
			(iter != _buffer->_fragments.cbegin()))
		{
			toDo -= offset;
			--iter;
			offset = iter->getLength();
		}
		if (toDo > offset)
		{
			// read beyond bounds.  What to do?
		}
		offset -= toDo;
	}
	return (*iter)[offset];
}

bool Buffer::const_itr::operator==(const const_itr & rhs) const
{
	return ((_buffer == rhs._buffer) && (_fragmentIterator == rhs._fragmentIterator) && (_fragmentOffset == rhs._fragmentOffset));
}

bool Buffer::const_itr::operator!=(const const_itr & rhs) const
{
	return !(*this==rhs);
}

//Buffer::const_itr::difference_type Buffer::const_itr::operator-(const const_itr & rhs) const
//{
//	difference_type result = 0;
//	if (&_buffer == &rhs._buffer)
//	{
//		difference_type offset = _fragmentOffset;
//		auto iter = _fragmentIterator;
//		while (_fragmentIterator < rhs._fragmentIterator)
//		{
//			result -= (iter->getLength() - offset);
//			++iter;
//			offset = 0;
//		}
//		while (_fragmentIterator > rhs._fragmentIterator)
//		{
//			result += offset;
//			--iter;
//			offset = iter->getLength();
//		}
//		result += (offset - rhs._fragmentOffset);
//	}
//	// else different buffers. ?
//	return result;
//}

Buffer::const_itr::difference_type operator-(const Buffer::const_itr & lhs, const Buffer::const_itr & rhs)
{
	Buffer::const_itr::difference_type result = 0;
	if (lhs._buffer == rhs._buffer)
	{
		Buffer::const_itr::difference_type offset = lhs._fragmentOffset;
		auto iter = lhs._fragmentIterator;
		while (iter < rhs._fragmentIterator)
		{
			result -= (iter->getLength() - offset);
			++iter;
			offset = 0;
		}
		while (iter > rhs._fragmentIterator)
		{
			result += offset;
			--iter;
			offset = iter->getLength();
		}
		result += (offset - rhs._fragmentOffset);
	}
	// else different buffers. ?
	return result;
}

Buffer::const_itr & Buffer::const_itr::operator+=(difference_type increase)
{
	if (increase > 0)
	{
		auto toDo = increase;
		while ((_fragmentIterator != _buffer->_fragments.cend()) &&
			(toDo >= (_fragmentIterator->getLength() - _fragmentOffset)))
		{
			toDo -= (_fragmentIterator->getLength() - _fragmentOffset);
			_fragmentOffset = 0;
			++_fragmentIterator;
		}

		if ((_fragmentIterator != _buffer->_fragments.cend()) &&
			(toDo < (_fragmentIterator->getLength() - _fragmentOffset)))
		{
			_fragmentOffset += toDo;
		}
	}
	else
	{
		auto toDo = -increase;
		while ((toDo > _fragmentOffset) &&
			(_fragmentIterator != _buffer->_fragments.cbegin()))
		{
			toDo -= _fragmentOffset;
			--_fragmentIterator;
			_fragmentOffset = _fragmentIterator->getLength();
		}
		if (toDo > _fragmentOffset)
		{
			// past start of buffer, so set to begin
			_fragmentOffset = 0;
		}
		else
		{
			_fragmentOffset -= toDo;
		}
	}
	return *this;
}

bool operator<(const Buffer::const_itr& lhs, const Buffer::const_itr& rhs)
{
	if (lhs._fragmentIterator == rhs._fragmentIterator)
	{
		return lhs._fragmentOffset < rhs._fragmentOffset;
	}
	else
	{
		return lhs._fragmentIterator < rhs._fragmentIterator;
	}
}

Buffer::const_itr & Buffer::const_itr::operator-=(difference_type decrease)
{
	return (*this) += -decrease;
}


std::string Buffer::asString() const
{
	std::string result("Buffer:\n");

	for (auto fragment : _fragments)
	{
		result += fragment.asString();
	}

	return result;
}