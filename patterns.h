#ifndef PATTERNS_H
#define PATTERNS_H

#ifdef _WIN32
#pragma once
#endif

#include <memutils/patterns.h>

namespace Patterns
{
	namespace Client
	{
		EXTERN_PATTERNS(CClient_SoundEngine__LoadSoundList);
		EXTERN_PATTERN(CClient_SoundEngine__FlushCache);
	}
}

#endif