/*
*  Copyright 2018 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * libclime: Text parsing utils
 *	@file		parseUtils.hpp
 *	@brief		Various text parsing utils that has not found a better home yet.
 ******************************************************************************/
#pragma once
#ifndef CLIME_PARSEUTILS_HPP
#define CLIME_PARSEUTILS_HPP

#include <solace/types.hpp>
#include <solace/string.hpp>
#include <solace/result.hpp>
#include <solace/error.hpp>


namespace clime {

Solace::Result<bool, Solace::Error> tryParseBoolean(Solace::StringView value) noexcept;

Solace::Result<Solace::int8, Solace::Error> tryParseInt8(Solace::StringView value) noexcept;

Solace::Result<Solace::int16, Solace::Error> tryParseInt16(Solace::StringView value) noexcept;

Solace::Result<Solace::int32, Solace::Error> tryParseInt32(Solace::StringView value) noexcept;

Solace::Result<Solace::int64, Solace::Error> tryParseInt64(Solace::StringView value) noexcept;

Solace::Result<Solace::uint8, Solace::Error> tryParseUInt8(Solace::StringView value) noexcept;

Solace::Result<Solace::uint16, Solace::Error> tryParseUInt16(Solace::StringView value) noexcept;

Solace::Result<Solace::uint32, Solace::Error> tryParseUInt32(Solace::StringView value) noexcept;

Solace::Result<Solace::uint64, Solace::Error> tryParseUInt64(Solace::StringView value) noexcept;


template<typename T>
Solace::Result<T, Solace::Error> tryParse(Solace::StringView value) noexcept;


template<>
inline
Solace::Result<bool, Solace::Error> tryParse<bool>(Solace::StringView value) noexcept { return tryParseBoolean(value); }

template<>
inline
Solace::Result<Solace::int8, Solace::Error>
tryParse<Solace::int8>(Solace::StringView value) noexcept { return tryParseInt8(value); }

template<>
inline
Solace::Result<Solace::int16, Solace::Error>
tryParse<Solace::int16>(Solace::StringView value) noexcept { return tryParseInt16(value); }

template<>
inline
Solace::Result<Solace::int32, Solace::Error>
tryParse<Solace::int32>(Solace::StringView value) noexcept { return tryParseInt32(value); }

template<>
inline
Solace::Result<Solace::int64, Solace::Error>
tryParse<Solace::int64>(Solace::StringView value) noexcept { return tryParseInt64(value); }


template<>
inline
Solace::Result<Solace::uint8, Solace::Error>
tryParse<Solace::uint8>(Solace::StringView value) noexcept { return tryParseUInt8(value); }

template<>
inline
Solace::Result<Solace::uint16, Solace::Error>
tryParse<Solace::uint16>(Solace::StringView value) noexcept { return tryParseUInt16(value); }

template<>
inline
Solace::Result<Solace::uint32, Solace::Error>
tryParse<Solace::uint32>(Solace::StringView value) noexcept { return tryParseUInt32(value); }

template<>
inline
Solace::Result<Solace::uint64, Solace::Error>
tryParse<Solace::uint64>(Solace::StringView value) noexcept { return tryParseUInt64(value); }


}  // End of namespace clime
#endif  // CLIME_PARSEUTILS_HPP
