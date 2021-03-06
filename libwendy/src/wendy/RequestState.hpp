/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2013 Remi Papillie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 *****************************************************************************/

#ifndef __WENDY_REQUESTSTATE_HPP__
#define __WENDY_REQUESTSTATE_HPP__

#include <wendy/common.hpp>

namespace wendy {

/**
 * \class RequestState
 * \brief Asynchronous call information block
 */
class WENDYAPI RequestState
{
	public:
		RequestState();
		~RequestState();
		
		bool isFinished() const;
		bool isSuccess() const;
		
		void start();
		void succeed();
		void fail();
		
	private:
		enum State
		{
			NOT_STARTED,
			IN_PROGRESS,
			SUCCEEDED,
			FAILED
		};
		State state;
};

} // wendy namespace

#endif // __WENDY_REQUESTSTATE_HPP__
