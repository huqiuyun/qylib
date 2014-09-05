#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include <windows.h>
#include <assert.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if( p ) { delete (p); (p) = 0;}
#endif

