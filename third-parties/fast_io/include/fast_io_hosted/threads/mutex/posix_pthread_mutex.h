﻿#pragma once

#include <pthread.h>

namespace fast_io
{

struct posix_pthread_mutex
{
	using native_handle_type = pthread_mutex_t;
	native_handle_type mutex;
	inline explicit posix_pthread_mutex() noexcept
		: mutex(PTHREAD_MUTEX_INITIALIZER)
	{}
	inline posix_pthread_mutex(posix_pthread_mutex const &) = delete;
	inline posix_pthread_mutex &operator=(posix_pthread_mutex const &) = delete;
	inline void lock()
	{
		if (noexcept_call(pthread_mutex_lock, __builtin_addressof(mutex))) [[unlikely]]
		{
			throw_posix_error();
		}
	}
	inline bool try_lock() noexcept
	{
		return !noexcept_call(pthread_mutex_trylock, __builtin_addressof(mutex));
	}
	inline void unlock() noexcept
	{
		noexcept_call(pthread_mutex_unlock, __builtin_addressof(mutex));
	}

	inline ~posix_pthread_mutex()
	{
		noexcept_call(pthread_mutex_destroy, __builtin_addressof(mutex));
	}
};

} // namespace fast_io
