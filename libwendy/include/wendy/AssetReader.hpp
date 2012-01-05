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

#ifndef __WENDY_ASSETREADER_HPP__
#define __WENDY_ASSETREADER_HPP__

#include <wendy/common.hpp>
#include <wendy/AssetNotification.hpp>
#include <wendy/Runnable.hpp>

namespace wendy {

class LocalStream;
class Thread;
template <class PayloadType> class Queue;

/**
 * \class AssetReader
 * \brief Downloads asset metadata asynchronously
 */
class WENDYAPI AssetReader: public Runnable
{
	public:
		AssetReader(LocalStream *stream);
		~AssetReader();
		
		virtual void run();
		
		/**
		 * \brief Return true if at least one asset notification information is available
		 */
		bool hasNotification();
		
		/**
		 * \brief Read one asset notification
		 * \return true if the stream was disconnected and no more notifications will arrive
		 * 
		 * Blocks if nothing is available, until a change information arrives
		 */
		bool getNextNotification(AssetNotification *notification);
		
	private:
		LocalStream *stream;
		Thread *thread;
		Queue<AssetNotification> *queue;
};

} // wendy namespace

#endif // __WENDY_ASSETREADER_HPP__
