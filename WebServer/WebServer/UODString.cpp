#include "stdafx.h"
#include "UODString.h"

class UODStringImplementation
{
public:
	UODStringImplementation()
	{

	}

	UODStringImplementation(char * s)
	{

	}

	UODStringImplementation(const UODStringImplementation& s)
	{

	}

	UODStringImplementation(UODString&& s)
	{

	}

	~UODStringImplementation()
	{

	}

	int Length() const
	{
		return strlen(_string);
	}

	UODStringImplementation& operator= (const UODStringImplementation& rhs)
	{

	}

	UODStringImplementation& operator= (UODStringImplementation&& rhs)
	{

	}

	char operator[] (const int index) const
	{

	}

	char& operator[] (const int index)
	{

	}

	const UODStringImplementation operator+(const UODStringImplementation &other) const
	{

	}

	bool operator== (const UODStringImplementation& rhs) const
	{
		if (rhs.Length() != strlen(_string))
		{
			return false;
		}

		int i = 0;
		int l = strlen(_string);
		for (int i = 0; i < l; i++)
		{
			if (_string[i] != rhs[i])
			{
				return false;
			}
		}

		return true;
	}

	UODStringImplementation SubString(const int start, const int length) const
	{
		UODStringImplementation newString = UODStringImplementation();
		for (int i = start; i < length + start; i++)
		{
		}
	}

	UODStringImplementation SubString(const int start) const
	{

	}

	int IndexOf(const char c) const
	{
		int l = strlen(_string);
		int i = 0;
		for (i = 0; i < l; i++)
		{
			if (_string[i] == c)
			{
				return i;
			}
		}

		return -1;
	}
private:
	char* _string;
};

UODString::UODString()
{
}


UODString::~UODString()
{
}
