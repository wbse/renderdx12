#pragma once
#ifdef USERENDERDX12
#define RENDERDX12DLL __declspec(dllimport)
#else
#define RENDERDX12DLL __declspec(dllexport)
#endif

#include "../../render/include/userender.h"