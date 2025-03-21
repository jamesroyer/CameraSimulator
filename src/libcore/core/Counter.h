#pragma once

#include <cstddef>

// Counter is used to track the number of objects (i.e. T) created.
template <typename T>
class Counter
{
	static size_t count;
public:
	Counter() { ++count; }
	Counter(const Counter&) { ++count; }
	~Counter() { --count; }
public:
	static size_t HowMany() { return count; }
};

template <typename T>
size_t Counter<T>::count {0};
