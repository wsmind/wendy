/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

#ifndef __WENDY_ASSETFILE_HPP__
#define __WENDY_ASSETFILE_HPP__

#include <wendy/common.hpp>

namespace wendy {

/**
 * \class AsstFile
 * \brief Abstract view on an open asset
 *
 * Open assets (either for reading or writing) can be manipulated with
 * a file-like interface, in order to acces their content.
 */
class WENDYAPI AssetFile
{
	public:
		enum OpenMode
		{
			READING,
			WRITING,
		};
		AssetFile(const std::string &filename, OpenMode mode);
		
		~AssetFile();
		
		/**
		 * \brief Try to read the given size
		 * \return the number of bytes actually read into the given buffer
		 */
		size_t read(void *buffer, size_t size);
		
		/**
		 * \brief Try to write the given size
		 * \return the number of bytes actually written into asset content
		 */
		size_t write(const void *buffer, size_t size);
		
		enum SeekReference
		{
			START,
			CURRENT,
			END
		};
		
		/**
		 * \brief Move the file pointer by offset bytes, relative to the given reference point
		 */
		int seek(SeekReference reference, int offset);
		
		/**
		 * \brief Retrieve the current file pointer position
		 */
		long tell();
};

} // wendy namespace

#endif // __WENDY_ASSETFILE_HPP__

