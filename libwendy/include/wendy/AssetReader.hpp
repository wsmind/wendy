#ifndef __WENDY_ASSETREADER_HPP__
#define __WENDY_ASSETREADER_HPP__

#include <wendy/common.hpp>

namespace wendy {

class LocalStream;

/**
 * \class AssetReader
 */
class WENDYAPI AssetReader
{
	public:
		AssetReader(LocalStream *stream);
		~AssetReader();
		
	private:
		LocalStream *stream;
};

} // wendy namespace

#endif // __WENDY_ASSETREADER_HPP__

