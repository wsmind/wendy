#ifndef __WENDY_H__
#define __WENDY_H__

// ensure that symbols are exported with C name mangling
#ifdef __cplusplus
extern "C" {
#endif

// MSVC needs __declspec to export symbols
#ifdef _MSC_VER
#	ifdef BUILDING_WENDY
#		define WENDYAPI __declspec(dllexport)
#	else
#		define WENDYAPI __declspec(dllimport)
#	endif
#else
#	define WENDYAPI
#endif

typedef struct
{
	unsigned long long id;
	char *path;
} WendyAsset;

// notifications about asset status
typedef void (*WendyAssetCallback)(const WendyAsset *asset);

void WENDYAPI wendyConnect(WendyAssetCallback callback);
void WENDYAPI wendyDisconnect();

void WENDYAPI wendyPollEvents(); // call the callback (in the caller thread) if some events arrived, or do nothing
void WENDYAPI wendyWaitEvents(); // block until some events arrived

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __WENDY_H__

