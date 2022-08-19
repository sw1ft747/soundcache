#include <Windows.h>
#include <string>

#include <IClientPlugin.h>
#include <IDetoursAPI.h>
#include <IMemoryUtils.h>
#include <IHooks.h>
#include <interface.h>

#include <hl_sdk/common/protocol.h>

#include <convar.h>
#include <dbg.h>

#include "patterns.h"

//-----------------------------------------------------------------------------
// Soundcache plugin interface
//-----------------------------------------------------------------------------

class CSoundcache : public IClientPlugin
{
public:
	virtual api_version_s GetAPIVersion();

	virtual bool Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers);

	virtual void PostLoad(bool bGlobalLoad);

	virtual void Unload(void);

	virtual bool Pause(void);

	virtual void Unpause(void);

	virtual void GameFrame(client_state_t state, double frametime, bool bPostRunCmd);

	virtual PLUGIN_RESULT Draw(void);

	virtual PLUGIN_RESULT DrawHUD(float time, int intermission);

	virtual const char *GetName(void);

	virtual const char *GetAuthor(void);

	virtual const char *GetVersion(void);

	virtual const char *GetDescription(void);

	virtual const char *GetURL(void);

	virtual const char *GetDate(void);

	virtual const char *GetLogTag(void);

private:
	void *m_pfnCClient_SoundEngine__LoadSoundList;

	DetourHandle_t m_hCClient_SoundEngine__LoadSoundList;
	DetourHandle_t m_hNetMsgHook_ResourceList;
};

//-----------------------------------------------------------------------------
// Declare Hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK(bool, CClient_SoundEngine__LoadSoundList, void *thisptr);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

NetMsgHookFn ORIG_NetMsgHook_ResourceList = NULL;

static char mapname_buffer[MAX_PATH];
static char szMapName[MAX_PATH];

static char szSoundcacheDir[MAX_PATH];
static char szServerSoundcacheDir[MAX_PATH];
static char szServerSoundcacheFolder[MAX_PATH];

bool bHasSoundcache = false;
bool paused = false;

//-----------------------------------------------------------------------------
// ConCommands, CVars..
//-----------------------------------------------------------------------------

ConVar autosave_soundcache("autosave_soundcache", "1", FCVAR_CLIENTDLL, "Automatically save soundcache");

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void HOOKED_NetMsgHook_ResourceList(void)
{
	Warning("ResourceList\n");

	if ( !autosave_soundcache.GetBool() || paused )
		return ORIG_NetMsgHook_ResourceList();

	netadr_t addr;
	int port;

	char mapname_buffer[MAX_PATH];

	char *pszMapName = mapname_buffer;
	char *pszExt = NULL;

	strncpy(mapname_buffer, g_pEngineFuncs->GetLevelName(), MAX_PATH);

	// maps/<mapname>.bsp to <mapname>
	while (*pszMapName)
	{
		if (*pszMapName == '/')
		{
			pszMapName++;
			break;
		}

		pszMapName++;
	}

	pszExt = pszMapName;

	while (*pszExt)
	{
		if (*pszExt == '.')
		{
			*pszExt = 0;
			break;
		}

		pszExt++;
	}

	g_pEngineClient->GetServerAddress(&addr);
	port = (addr.port << 8) | (addr.port >> 8); // it's swapped

	strncpy(szMapName, pszMapName, MAX_PATH);

	snprintf(szSoundcacheDir, MAX_PATH, "maps\\soundcache\\%s.txt", szMapName);
	snprintf(szServerSoundcacheDir, MAX_PATH, "maps\\soundcache\\%hhu.%hhu.%hhu.%hhu %hu\\%s.txt", addr.ip[0], addr.ip[1], addr.ip[2], addr.ip[3], port, szMapName);
	snprintf(szServerSoundcacheFolder, MAX_PATH, "maps\\soundcache\\%hhu.%hhu.%hhu.%hhu %hu", addr.ip[0], addr.ip[1], addr.ip[2], addr.ip[3], port);

	// Uh we have the soundcache that wasn't deleted
	if ( g_pFileSystem->FileExists(szSoundcacheDir) )
	{
		std::string sDirectory = SvenModAPI()->GetBaseDirectory();

		SetFileAttributesA((sDirectory + "\\svencoop_downloads\\" + szSoundcacheDir).c_str(), FILE_ATTRIBUTE_NORMAL); // WinAPI
		//DeleteFileA(szFullSoundcacheDir);
		g_pFileSystem->RemoveFile(szSoundcacheDir, "GAMEDOWNLOAD");
	}

	// If client already has the saved soundcache then create a dummy file so we won't download anything..
	if ( g_pFileSystem->FileExists(szServerSoundcacheDir) )
	{
		FileHandle_t hFile = g_pFileSystem->Open(szSoundcacheDir, "a+", "GAMEDOWNLOAD"); // dummy

		if ( hFile )
		{
			g_pFileSystem->Close(hFile);
		}

		bHasSoundcache = true;
	}
	else
	{
		bHasSoundcache = false;
	}

	ORIG_NetMsgHook_ResourceList();
}

DECLARE_CLASS_FUNC(bool, HOOKED_CClient_SoundEngine__LoadSoundList, void *thisptr)
{
	Warning("LoadSoundList\n");

	if ( !autosave_soundcache.GetBool() || paused )
		return ORIG_CClient_SoundEngine__LoadSoundList(thisptr);

	if ( g_pFileSystem->FileExists(szSoundcacheDir) )
	{
		std::string sDirectory = SvenModAPI()->GetBaseDirectory();
		std::string sSoundcacheDir = sDirectory + "\\svencoop_downloads\\" + szSoundcacheDir;
		std::string sServerSoundcacheDir = sDirectory + "\\svencoop_downloads\\" + szServerSoundcacheDir;

		SetFileAttributesA(sSoundcacheDir.c_str(), FILE_ATTRIBUTE_NORMAL);

		if ( !CreateDirectoryA((sDirectory + "\\svencoop_downloads\\" + szServerSoundcacheFolder).c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			Warning("Failed to create directory \"..\\%s\"\n", szServerSoundcacheFolder);
			return ORIG_CClient_SoundEngine__LoadSoundList(thisptr);
		}

		if ( bHasSoundcache )
		{
			g_pFileSystem->RemoveFile(szSoundcacheDir, "GAMEDOWNLOAD");

			if ( CopyFileA(sServerSoundcacheDir.c_str(), sSoundcacheDir.c_str(), true) )
			{
				Msg("Used saved soundcache for the current map \"%s\"\n", szMapName);
			}
			else
			{
				Warning("Failed to replace soundcache for the current map \"%s\"\n", szMapName);
			}
		}
		else
		{
			if ( CopyFileA(sSoundcacheDir.c_str(), sServerSoundcacheDir.c_str(), true) )
			{
				Msg("Saved soundcache for the current map \"%s\"\n", szMapName);
			}
			else
			{
				Warning("Failed to save soundcache for the current map \"%s\"\n", szMapName);
			}
		}
	}
	else
	{
		Warning("Unable to find soundcache for the current map \"%s\"\n", szMapName);
	}

	return ORIG_CClient_SoundEngine__LoadSoundList(thisptr);
}
//-----------------------------------------------------------------------------
// Plugin's implementation
//-----------------------------------------------------------------------------

api_version_s CSoundcache::GetAPIVersion()
{
	return SVENMOD_API_VER;
}

bool CSoundcache::Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers)
{
	BindApiToGlobals(pSvenModAPI);

	m_pfnCClient_SoundEngine__LoadSoundList = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Client, Patterns::Client::CClient_SoundEngine__LoadSoundList );

	if ( !m_pfnCClient_SoundEngine__LoadSoundList )
	{
		Warning("Couldn't find function \"CClient_SoundEngine::LoadSoundList\"\n");
		Warning("[Soundcache] Failed to initialize\n");

		return false;
	}

	ConVar_Register();

	ConColorMsg({ 40, 255, 40, 255 }, "[Soundcache] Successfully loaded\n");
	return true;
}

void CSoundcache::PostLoad(bool bGlobalLoad)
{
	if (bGlobalLoad)
	{
	}
	else
	{
	}

	m_hNetMsgHook_ResourceList = Hooks()->HookNetworkMessage( SVC_RESOURCELIST,
															 HOOKED_NetMsgHook_ResourceList,
															 &ORIG_NetMsgHook_ResourceList );

	m_hCClient_SoundEngine__LoadSoundList = DetoursAPI()->DetourFunction( m_pfnCClient_SoundEngine__LoadSoundList,
																		 HOOKED_CClient_SoundEngine__LoadSoundList,
																		 GET_FUNC_PTR(ORIG_CClient_SoundEngine__LoadSoundList) );
}

void CSoundcache::Unload(void)
{
	DetoursAPI()->RemoveDetour( m_hNetMsgHook_ResourceList );
	DetoursAPI()->RemoveDetour( m_hCClient_SoundEngine__LoadSoundList );

	ConVar_Unregister();
}

bool CSoundcache::Pause(void)
{
	paused = true;
	return true;
}

void CSoundcache::Unpause(void)
{
	paused = false;
}

void CSoundcache::GameFrame(client_state_t state, double frametime, bool bPostRunCmd)
{
	if (bPostRunCmd)
	{
	}
	else
	{
	}
}

PLUGIN_RESULT CSoundcache::Draw(void)
{
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CSoundcache::DrawHUD(float time, int intermission)
{
	return PLUGIN_CONTINUE;
}

const char *CSoundcache::GetName(void)
{
	return "Soundcache";
}

const char *CSoundcache::GetAuthor(void)
{
	return "Sw1ft";
}

const char *CSoundcache::GetVersion(void)
{
	return "1.0.0";
}

const char *CSoundcache::GetDescription(void)
{
	return "Automatically saves soundcache";
}

const char *CSoundcache::GetURL(void)
{
	return "https://github.com/sw1ft747/";
}

const char *CSoundcache::GetDate(void)
{
	return SVENMOD_BUILD_TIMESTAMP;
}

const char *CSoundcache::GetLogTag(void)
{
	return "SNDC";
}

//-----------------------------------------------------------------------------
// Export the interface
//-----------------------------------------------------------------------------

EXPOSE_SINGLE_INTERFACE(CSoundcache, IClientPlugin, CLIENT_PLUGIN_INTERFACE_VERSION);