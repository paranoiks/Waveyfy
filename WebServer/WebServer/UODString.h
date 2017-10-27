#pragma once
#include "stdafx.h"

class UODStringImplementation;

class UODString
{
	friend std::ostream& operator<<(std::ostream& output, const UODString& p);
public:
	UODString();
	UODString(char * s);
	UODString(const UODString& s);
	UODString(UODString&& s);
	~UODString();
	int Length() const;
	UODString& operator= (const UODString& rhs);
	UODString& operator= (UODString&& rhs);
	char operator[] (const int index) const;
	char& operator[] (const int index);
	const UODString operator+(const UODString &other) const;
	bool operator== (const UODString& rhs) const;
	UODString SubString(const int start, const int length) const;
	UODString SubString(const int start) const;
	int IndexOf(const char c) const;
private:
	UODStringImplementation* _uodStringImplementation;
};

