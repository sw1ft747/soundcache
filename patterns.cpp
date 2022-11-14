#include "patterns.h"

namespace Patterns
{
	namespace Client
	{
		DEFINE_PATTERN(CClient_SoundEngine__LoadSoundList, "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 A8 08 00 00");
		DEFINE_PATTERN(CClient_SoundEngine__FlushCache, "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 54 03 00 00");
	}
}