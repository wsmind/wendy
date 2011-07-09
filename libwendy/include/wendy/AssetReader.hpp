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
		 * 
		 * Blocks if nothing is available, until a change information arrives
		 */
		AssetNotification getNextNotification();
		
	private:
		LocalStream *stream;
		Thread *thread;
		Queue<AssetNotification> *queue;
};

} // wendy namespace

#endif // __WENDY_ASSETREADER_HPP__

