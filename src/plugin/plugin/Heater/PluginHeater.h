#pragma once

#include <plugin/IPlugin.h>

extern "C"
{

const char* name();
const char* version();

// IPlugin* create(Callbacks* cb)
void* create(void* ptr);

// void destroy(IPlugin* ptr)
void destroy(void* ptr);

}
