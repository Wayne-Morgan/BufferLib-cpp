#pragma once

#include <vector>
#include <memory>
#include "BufferFragment.h"

class IMemoryBlock;

	class Buffer
	{
	public:
		// Buffer construction
		explicit Buffer(std::shared_ptr<IMemoryBlock> pMemoryBlock);

		class const_itr : public std::iterator<std::random_access_iterator_tag, char>
		{
		public:
//			using difference_type = typename std::iterator<std::random_access_iterator_tag, char>::difference_type;
			typedef std::iterator<std::random_access_iterator_tag, char>::difference_type difference_type;

			const_itr(const Buffer& xBuffer);

			const_itr& operator++();
			const_itr operator++(int);
			const_itr& operator--();
			const_itr operator--(int);
			const char& operator*() const;
			const char& operator[](difference_type xIndex) const;
			const char* operator->() const;
			bool operator==(const const_itr& rhs) const;
			bool operator!=(const const_itr& rhs) const;
			//difference_type operator-(const const_itr& rhs) const;
			//const_itr operator-(difference_type decrease) const;
			//const_itr operator+(difference_type increase) const;
			friend const_itr operator+(const const_itr& lhs, difference_type rhs);
			friend const_itr operator+(difference_type lhs, const const_itr& rhs);
			friend const_itr operator-(const const_itr& lhs, difference_type rhs);
			friend difference_type operator-(const const_itr& lhs, const const_itr& rhs);
			const_itr& operator+=(difference_type increase);
			const_itr& operator-=(difference_type decrease);
			friend bool operator<(const const_itr& lhs, const const_itr& rhs);
			friend bool operator>(const const_itr& lhs, const const_itr& rhs);
			friend bool operator<=(const const_itr& lhs, const const_itr& rhs);
			friend bool operator>=(const const_itr& lhs, const const_itr& rhs);
		private:
			const Buffer* _buffer;
			std::vector<BufferFragment>::const_iterator _fragmentIterator;
			difference_type _fragmentOffset;
		};


		// Construct sub-buffer (cut off start)
		Buffer(const Buffer& srcBuffer, const const_itr& copyFrom);
		// Construct sub-buffer (cut start and end)
		Buffer(const Buffer& srcBuffer, const const_itr& copyFrom, const const_itr& copyTo);

		// Buffer concatenation
		Buffer& operator+=(const Buffer& srcBuffer);

		const_itr cbegin() const;
		const_itr cend() const;


		size_t getLength() const;
		const char& operator[](size_t offset) const;
		size_t copy(size_t offset, size_t length, char* pDestination) const;
		// return the address of a char at a given offset into the buffer,
		// and the size of the contiguous buffer memory from this offset
		//const char* getContiguous(size_t offset, size_t* length);
		std::string asString() const;
	private:
		std::vector<BufferFragment> _fragments;

	};


	// Buffer concatenate and create
	Buffer operator+(const Buffer& lhs, const Buffer& rhs);
	Buffer::const_itr::difference_type operator-(const Buffer::const_itr & lhs, const Buffer::const_itr & rhs);

