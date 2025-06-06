﻿#pragma once

namespace fast_io
{

namespace details
{

template <::std::integral char_type>
inline constexpr ::std::size_t dummy_cstr_len(char_type const *cstr) noexcept
{
	::std::size_t n{};
	for (; *cstr; ++cstr)
	{
		++n;
	}
	return n;
}

template <::std::integral char_type>
inline constexpr ::std::size_t dummy_cstr_nlen(char_type const *cstr, ::std::size_t maxn) noexcept
{
	::std::size_t n{};
	for (; n != maxn && cstr[n]; ++n)
		;
	return n;
}

} // namespace details

template <::std::integral char_type>
inline constexpr ::std::size_t cstr_len(char_type const *cstr) noexcept
{
	if (__builtin_is_constant_evaluated())
	{
		return details::dummy_cstr_len(cstr);
	}
	else
	{
		if constexpr (::std::same_as<char_type, char>)
		{
#if FAST_IO_HAS_BUILTIN(__builtin_strlen)
			return __builtin_strlen(cstr);
#else
			return ::std::strlen(cstr);
#endif
		}
		else
		{
			return details::dummy_cstr_len(cstr);
		}
	}
}

template <::std::integral char_type>
inline constexpr ::std::size_t cstr_nlen(char_type const *cstr, ::std::size_t n) noexcept
{
	if (__builtin_is_constant_evaluated())
	{
		return details::dummy_cstr_nlen(cstr, n);
	}
	else
	{
		if constexpr (::std::same_as<char_type, char>)
		{
#if FAST_IO_HAS_BUILTIN(__builtin_strnlen)
			return __builtin_strnlen(cstr, n);
#else
			return details::dummy_cstr_nlen(cstr, n);
#endif
		}
		else
		{
			return details::dummy_cstr_nlen(cstr, n);
		}
	}
}

} // namespace fast_io
