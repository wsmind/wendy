#ifndef __WENDY_COMMON_HPP__
#define __WENDY_COMMON_HPP__

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

#endif // __WENDY_COMMON_HPP__

