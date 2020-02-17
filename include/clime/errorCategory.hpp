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
#pragma once
#ifndef CLIME_ERRORCATEGORY_HPP
#define CLIME_ERRORCATEGORY_HPP

#include <solace/error.hpp>

namespace clime {

using Error = Solace::Error;

/// Error category for parser specific errors
extern Solace::AtomValue const kParserErrorCatergory;


/// Error 'codes'
enum class ParserError : int {
	NoError = 0,			/// Not a real error -used when 'help' is requested.
	InvalidNumberOfArgs = 1,  /// Odd-error: null string in arguments list

	ValueExpected,
	UnexpectedValue,

	InvalidInput,
	OptionParsing,
};


Error
makeParserError(ParserError errorCode, Solace::StringView tag) noexcept;


}  // End of namespace clime
#endif  // CLIME_ERRORCATEGORY_HPP
