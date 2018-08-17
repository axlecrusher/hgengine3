#ifndef CALLBACK_H
#define CALLBACK_H

/// A simple callback template library typesafe at compile time
class Callback0
{
public:
	Callback0(void(*f)(void))
		:functor(f)
	{}

	void operator()(void) const
	{
		functor();
	}

private:
	void (*functor)(void);
};

template <typename R>
class Callback0R
{
public:
	Callback0R(R(*f)(void))
		:functor(f)
	{}

	R operator()(void) const
	{
		return functor();
	}

private:
	R (*functor)(void);
};

template <typename P1, typename R1>
		class Callback1R
{
	public:
		Callback1R(R1(*f)(P1))
	:functor(f)
		{}

		R1 operator()(P1 p1) const
		{
			return functor(p1);
		}

	private:
		R1 (*functor)(P1);
};


template <typename P1>
class Callback1
{
public:
	Callback1(void(*f)(P1))
		:functor(f)
	{}

	void operator()(P1 p1) const
	{
		functor(p1);
	}

private:
	void (*functor)(P1);
};

template <typename P1, typename P2>
class Callback2
{
public:
	Callback2(void(*f)(P1,P2))
		:functor(f)
	{}

	void operator()(P1 p1, P2 p2) const
	{
		functor(p1, p2);
	}

private:
	void (*functor)(P1, P2);
};

template <typename P1, typename P2, typename P3>
class Callback3
{
public:
	Callback3(void(*f)(P1,P2,P3))
		:functor(f)
	{}

	void operator()(P1 p1, P2 p2, P3 p3) const
	{
		functor(p1, p2, p3);
	}

private:
	void (*functor)(P1, P2, P3);
};

template <typename P1, typename P2, typename P3, typename P4>
class Callback4
{
public:
	Callback4(void(*f)(P1,P2,P3,P4))
		:functor(f)
	{}

	void operator()(P1 p1, P2 p2, P3 p3,P4 p4) const
	{
		functor(p1, p2, p3, p4);
	}

private:
	void (*functor)(P1, P2, P3, P4);
};

#endif

/* Copyright (c) 2008, Joshua Allen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *	-	Redistributions of source code must retain the above
 *		copyright notice, this list of conditions and the following disclaimer.
 *	-	Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in
 *		the documentation and/or other materials provided with the distribution.
 *	-	Neither the name of the Mercury Engine nor the names of its
 *		contributors may be used to endorse or promote products derived from
 *		this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
