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
 * @file: parseUtils.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "clime/parseUtils.hpp"
#include "clime/errorCategory.hpp"

#include <cstring>
#include <cstdlib>
#include <sstream>      // std::stringstream, std::stringbuf

#include <type_traits>  // std::is_signed
#include <climits>
#include <limits>



using namespace Solace;
using namespace clime;

namespace /* anonymous */ {

auto conversionError(StringLiteral mgs, StringView) noexcept {
	return makeParserError(ParserError::OptionParsing, mgs);
}


template<typename T, bool isSigned = std::is_signed<T>::value>
struct Longest { };


template<typename T>
struct Longest<T, true> {
    using type = int64;  // long long;

	static Result<T, Error> parse(StringView value) noexcept {
        errno = 0;
        char* pEnd = nullptr;
        // FIXME(abbyssoul): The use of value.data() here is not safe for substrings.
        auto const result = strtoll(value.data(), &pEnd, 0);

        if ((errno == ERANGE && (result == LLONG_MAX || result == LLONG_MIN)) || (errno != 0 && result == 0)) {
			return conversionError("Value is outside of int64 range", value);
        }

		if (!pEnd || pEnd == value.data()) {  // No conversion has been done
			return conversionError("Not a valid integer", value);
		}

        if (result > std::numeric_limits<T>::max() ||
            result < std::numeric_limits<T>::min())
			return conversionError("Value is outside of range", value);

		return Ok(narrow_cast<T>(result));
	}
};


template<typename T>
struct Longest<T, false> {
    using type = uint64;  // unsigned long long;

	static Result<T, Error> parse(StringView value) noexcept {
        errno = 0;
        char* pEnd = nullptr;
        // FIXME(abbyssoul): The use of value.data() here is not safe for substrings.
        auto const result = strtoull(value.data(), &pEnd, 0);

        if ((errno == ERANGE && (result == ULLONG_MAX)) || (errno != 0 && result == 0)) {
			return conversionError("Value is outside of uint64 range", value);
        }

		if (!pEnd || pEnd == value.data()) {  // No conversion has been done
			return conversionError("Not a valid integer", value);
		}

        if (result > std::numeric_limits<T>::max() ||
            result < std::numeric_limits<T>::min())
			return conversionError("Value is outside of range", value);

		// Note: Cast is safe as value range is checked above
		return Ok(narrow_cast<T>(result));
    }
};


}  // namespace


Result<bool, Error>
clime::tryParseBoolean(StringView value) noexcept {
    if (value.equals("1") || strncasecmp(value.data(), "true",
                                         std::min<size_t>(value.length(), 4)) == 0) {
        return Ok(true);
    }

    if (value.equals("0") || strncasecmp(value.data(), "false",
                                        std::min<size_t>(value.length(), 5)) == 0) {
        return Ok(false);
    }

	return conversionError("Not a valid boolean value", value);
}

Result<int8, Error>
clime::tryParseInt8(StringView value) noexcept { return Longest<int8>::parse(value); }

Result<int16, Error>
clime::tryParseInt16(StringView value) noexcept { return Longest<int16>::parse(value); }

Result<int32, Error>
clime::tryParseInt32(StringView value) noexcept { return Longest<int32>::parse(value); }

Result<int64, Error>
clime::tryParseInt64(StringView value) noexcept { return Longest<int64>::parse(value); }

Result<uint8, Error>
clime::tryParseUInt8(StringView value) noexcept { return Longest<uint8>::parse(value); }

Result<uint16, Error>
clime::tryParseUInt16(StringView value) noexcept { return Longest<uint16>::parse(value); }

Result<uint32, Error>
clime::tryParseUInt32(StringView value) noexcept { return Longest<uint32>::parse(value); }

Result<uint64, Error>
clime::tryParseUInt64(StringView value) noexcept { return Longest<uint64>::parse(value); }
