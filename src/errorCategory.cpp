/*
*  Copyright 2016 Ivan Ryabov
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
#include "clime/errorCategory.hpp"

#include <solace/string.hpp>

using namespace Solace;
using namespace clime;


/// Atom for Parser error category
const AtomValue clime::kParserErrorCatergory = atom("clime");


namespace /*anonimous*/ {

struct ParserErrorDomain final : public ErrorDomain {

	StringView name() const noexcept override { return "CLI arguments"; }

	String message(int errCode) const noexcept override {
		ParserError const code = static_cast<ParserError>(errCode);

		return makeString(codeToMessage(code));
	}


	StringLiteral codeToMessage(ParserError code) const noexcept {
		switch (code) {
		case ParserError::NoError:				return "not an error";
		case ParserError::InvalidNumberOfArgs:	return " invalid number of arguments";
		case ParserError::ValueExpected:		return " value is expected";
		case ParserError::UnexpectedValue:		return " unexpected value";
		case ParserError::InvalidInput:			return " invalid input";
		case ParserError::OptionParsing:		return " error parsing option value";
		}

		return "unknown error";
	}
};

ParserErrorDomain kParserErrorDomain{};

auto const rego_generic = registerErrorDomain(kParserErrorCatergory, kParserErrorDomain);

}  // namespace


Error
clime::makeParserError(ParserError errorCode, StringView tag) noexcept {
	return Error{kParserErrorCatergory, static_cast<int>(errorCode), tag};
}
