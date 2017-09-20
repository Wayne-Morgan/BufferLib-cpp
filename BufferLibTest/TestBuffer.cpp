#include "stdafx.h"
#include "CppUnitTest.h"
#include "Buffer.h"
#include "IMemoryBlock.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
std::ostringstream gDebug;

// Todo: Move BufferLib into namespace
// Todo: Make thread-safe
// Todo: Replace offsets and Lengths with iterators
// Todo: Add some usable concrete MemoryBlock classes
// Todo: Maybe replace pointer in iterator with weak_ptr
// Todo: Maybe add GTest/GMock
// Todo: Clean-up.  Too many functions, and too much repetition.  Write functions in terms of others

class TestMemoryBlock : public IMemoryBlock
{
public:
	TestMemoryBlock(const char* pContents) : _pContents{ pContents } { ++_ctorCount; }
	virtual ~TestMemoryBlock() { ++_dtorCount; }

	// Inherited via IMemoryBlock
	virtual const char * getMemory() const override { return &_pContents[0]; }
	virtual size_t getLength() const override { return strlen(_pContents); }
	virtual size_t copy(size_t sourceOffset, size_t sourceLength, char * pDestination) const override
	{
		size_t toCopy = ((getLength() - sourceOffset) < sourceLength) ? getLength() - sourceOffset : sourceLength;
		memcpy(pDestination, &_pContents[sourceOffset], toCopy);
		return toCopy;
	}
	virtual const char & operator[](size_t offset) const override { return _pContents[offset]; }

	static int _ctorCount;
	static int _dtorCount;
private:
	const char* _pContents;
	//static const char _contents[];
};
//const char TestMemoryBlock::_contents[] = "0123456789";
int TestMemoryBlock::_ctorCount = 0;
int TestMemoryBlock::_dtorCount = 0;

const char testContents[] = "0123456789";

	TEST_CLASS(BufferTest)
	{
	public:
		
		TEST_METHOD(ConstructorDestructor)
		{
			TestMemoryBlock::_ctorCount = 0;
			TestMemoryBlock::_dtorCount = 0;

			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Assert::AreEqual(TestMemoryBlock::_ctorCount, 1);
			Assert::AreEqual(TestMemoryBlock::_dtorCount, 0);

			{
				Buffer buffer(pBlock);
				pBlock.reset();
				// pBlock dtor not run until buffer destroyed
				Assert::AreEqual(TestMemoryBlock::_dtorCount, 0);
			}
			Assert::AreEqual(TestMemoryBlock::_dtorCount, 1);
		}

		TEST_METHOD(CopyConstructor)
		{
			TestMemoryBlock::_ctorCount = 0;
			TestMemoryBlock::_dtorCount = 0;

			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Assert::AreEqual(TestMemoryBlock::_ctorCount, 1);
			Assert::AreEqual(TestMemoryBlock::_dtorCount, 0);

			{
				Buffer buffer(pBlock);
				pBlock.reset();
				{
					Buffer buffer2{ buffer };
					//Assert::AreEqual(buffer2.getLength(), 10);
					Assert::AreEqual(TestMemoryBlock::_dtorCount, 0);
				}
				// pBlock ctor not run until buffer destroyed
				Assert::AreEqual(TestMemoryBlock::_dtorCount, 0);
			}
			Assert::AreEqual(TestMemoryBlock::_dtorCount, 1);
		}

		TEST_METHOD(SingleFragmentLength)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Logger::WriteMessage(gDebug.str().c_str());
			Assert::AreEqual(static_cast<int>(buffer.getLength()), 10);
		}

		TEST_METHOD(GetChar)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Assert::AreEqual(buffer[2], '2');
		}

		TEST_METHOD(SubBuffer)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			Assert::AreEqual((int)buffer2.getLength(), 5);
			Assert::AreEqual(buffer2[2], '4');
		}

		TEST_METHOD(ConcatenateBuffers)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			Buffer buffer3 = buffer + buffer2;
			Assert::AreEqual((int)buffer3.getLength(), 15);
			Assert::AreEqual(buffer3[14], '6');
		}

		TEST_METHOD(ConcatenateSelf)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			buffer += buffer2;
			Assert::AreEqual((int)buffer.getLength(), 15);
			Assert::AreEqual(buffer[14], '6');
		}

		TEST_METHOD(Copy)
		{
			const char expected[] = "012345678923456";
			char actual[16];
			memset(actual, 0, 16);
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			buffer += buffer2;

			buffer.copy(0, buffer.getLength(), actual);

			Assert::AreEqual(strcmp(actual, expected), 0);
		}

		TEST_METHOD(CrossFragSubBuffer)
		{
			const char expected[] = "6789234";
			char actual[16];
			memset(actual, 0, 16);
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			buffer += buffer2;

			Buffer buffer3(buffer, 6, 7);

			Assert::AreEqual((int)buffer3.getLength(), 7);
			buffer3.copy(0, buffer3.getLength(), actual);
			Assert::AreEqual(strcmp(actual, expected), 0);
		}

		TEST_METHOD(Iterate)
		{
			const char expected[] = "6789234";
			char actual[128];
			memset(actual, 0, sizeof(actual));
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Buffer buffer2(buffer, 2, 5);
			buffer += buffer2;
			const Buffer buffer3(buffer, 6, 7);
			char* pDest = actual;

			// Would like to use range based for but need non-const iterators
			//for (const auto buffchar : buffer3)
			//{
			//	*pDest++ = buffchar;
			//}
			auto itr = buffer3.cbegin();
			while (itr != buffer3.cend())
			{
				*pDest++ = *itr++;
			}

			Assert::AreEqual(strcmp(actual, expected), 0);
		}

		TEST_METHOD(find)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>("abcdefghijklmnopqrstuvwxyz") };
			Buffer buffer(pBlock);
			std::string searchTerm{ "jkl" };

			auto foundItr = std::search(buffer.cbegin(), buffer.cend(), searchTerm.cbegin(), searchTerm.cend());

			Assert::IsFalse(foundItr == buffer.cend());
			Assert::AreEqual(*foundItr, 'j');
			Assert::AreEqual(foundItr - buffer.cbegin(), 'j' - 'a');
		}

		TEST_METHOD(iteratorDifference)
		{
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>(testContents) };
			Buffer buffer(pBlock);
			Assert::AreEqual(10, buffer.cend() - buffer.cbegin());
		}

		TEST_METHOD(cutOutAllVowels)
		{
			// A more complex example.  Repeatedly find something within a buffer, split the 
			// buffer into buffers found before and after, then join them together.
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>("abcdefghijklmnopqrstuvwxyz") };
			std::string vowels{ "aeiou" };
			std::string expected{ "bcdfghjklmnpqrstvwxyz" };

			Buffer buffer(pBlock);
			auto firstVowel = std::find_first_of(buffer.cbegin(), buffer.cend(), vowels.cbegin(), vowels.cend());
			while (firstVowel != buffer.cend())
			{
				size_t offset{ static_cast<size_t>(firstVowel - buffer.cbegin()) };
				if (offset == 0)
				{
					buffer = Buffer{ buffer, 1 };
				}
				else if (offset + 1 == buffer.getLength())
				{
					buffer = Buffer{ buffer, 0, buffer.getLength() - 1 };
				}
				else
				{
					Buffer before{ buffer, 0, offset };
					Buffer after{ buffer, offset + 1 };
					buffer = before + after;
				}

				firstVowel = std::find_first_of(buffer.cbegin(), buffer.cend(), vowels.cbegin(), vowels.cend());
			}

			Assert::IsTrue(std::equal(buffer.cbegin(), buffer.cend(), expected.cbegin(), expected.cend()));
		}

		TEST_METHOD(replaceAllVowels)
		{
			// More complex example indicating how to change a buffer without changing the memory
			// repeatedly find something within a buffer, replace the found bit with another buffer
			std::shared_ptr<IMemoryBlock> pBlock{ std::make_shared<TestMemoryBlock>("abcdefghijklmnopqrstuvwxyz") };
			std::shared_ptr<IMemoryBlock> pInsertion{ std::make_shared<TestMemoryBlock>("-*-") };
			std::string vowels{ "aeiou" };  // Search for any vowel
			std::string expected{ "-*-bcd-*-fgh-*-jklmn-*-pqrst-*-vwxyz" }; // Each vowel replaced with "-*-"

			Buffer buffer(pBlock);  // Buffer to search
			Buffer insertion(pInsertion); // Buffer to addin place of search term
			auto firstVowel = std::find_first_of(buffer.cbegin(), buffer.cend(), vowels.cbegin(), vowels.cend());
			while (firstVowel != buffer.cend())
			{
				size_t offset{ static_cast<size_t>(firstVowel - buffer.cbegin()) };
				if (offset == 0)
				{
					buffer = insertion + Buffer{ buffer, 1 };
				}
				else if (offset + 1 == buffer.getLength())
				{
					buffer = Buffer{ buffer, 0, buffer.getLength() - 1 } + insertion;
				}
				else
				{
					Buffer before{ buffer, 0, offset };
					Buffer after{ buffer, offset + 1 };
					buffer = before + insertion + after;
				}

				firstVowel = std::find_first_of(buffer.cbegin(), buffer.cend(), vowels.cbegin(), vowels.cend());
			}

			Assert::IsTrue(std::equal(buffer.cbegin(), buffer.cend(), expected.cbegin(), expected.cend()));
		}

	};


