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
/*******************************************************************************
 * @file: clime/arguments.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "clime/parser.hpp"
#include "clime/parseUtils.hpp"


using namespace Solace;
using namespace clime;


namespace /* anonymous */ {

Optional<Error>
formatOptionalError(char const* SOLACE_UNUSED(inputType),
					StringView name,
					const char* SOLACE_UNUSED(expectedType),
					StringView SOLACE_UNUSED(value)) noexcept {
	return makeParserError(ParserError::OptionParsing, name);
}


template<typename T>
Optional<Error>
parseIntArgument(T* dest, StringView value, Parser::Context const& cntx) {
	auto val = tryParse<T>(value);
    if (val) {
        *dest = static_cast<T>(val.unwrap());
        return none;
    }

	return makeParserError(ParserError::OptionParsing, cntx.name);  // val.moveError();
}



Optional<Error>
parseBoolean(bool* dest, StringView value) {
    auto val = tryParse<bool>(value);
    if (val) {
        *dest = val.unwrap();
        return none;
    }

	return val.moveError();
}


}  // anonymous namespace


Parser::Option::Option(std::initializer_list<StringLiteral> arg, StringLiteral desc, StringView* dest)
	: Option{arg, desc, ArgumentValue::Required,
		[dest](Optional<StringView> const& value, Context const&) -> Optional<Error> {
            *dest = value.get();

            return none;
		}}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> arg, StringLiteral desc, int8* dest)
	: Option{arg, desc, ArgumentValue::Required,
        [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
		}}
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint8* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int16* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint16* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int32* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint32* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int64* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint64* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) {
                 return parseIntArgument(dest, value.get(), context);
			 }}
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, float32* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) -> Optional<Error> {
                char* pEnd = nullptr;
                // FIXME(abbyssoul): not safe use of data
                auto val = strtof(value.get().data(), &pEnd);

                if (!pEnd || pEnd == value.get().data()) {  // No conversion has been done
					return formatOptionalError("Option", context.name, "float32", value.get());
                }

                *dest = static_cast<float32>(val);

                return none;
			}}
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, float64* dest)
	: Option{names, desc, ArgumentValue::Required,
             [dest](Optional<StringView> const& value, Context const& context) -> Optional<Error> {
                char* pEnd = nullptr;
                // FIXME(abbyssoul): not safe use of data
                auto val = strtod(value.get().data(), &pEnd);

                if (!pEnd || pEnd == value.get().data()) {  // No conversion has been done
					return formatOptionalError("Option", context.name, "float64", value.get());
                }

                *dest = static_cast<float64>(val);

                return none;
			 }}
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, bool* dest)
	: Option{names, desc, ArgumentValue::Optional,
             [dest](Optional<StringView> const& value, Context const&) -> Optional<Error> {
				if (value) {
					return parseBoolean(dest, *value);
                }

                *dest = true;

                return none;
			}}
{
}



Parser::Argument::Argument(StringLiteral name, StringLiteral description, int8* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}
{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint8* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}
{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int16* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint16* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int32* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint32* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int64* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint64* dest)
	: Argument{name, description,
			   [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); }}

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, float32* dest)
	: Argument{name, description,
                [dest](StringView value, Context const& context) {
                    char* pEnd = nullptr;
                    // FIXME(abbyssoul): not safe use of data
                    *dest = strtof(value.data(), &pEnd);

                    return (!pEnd || pEnd == value.data())
							   ? formatOptionalError("Argument", context.name, "float32", value)
                               : none;
			   }}
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, float64* dest)
	: Argument{name, description,
                [dest](StringView value, Context const& context) {
                    char* pEnd = nullptr;
                    // FIXME(abbyssoul): not safe use of data
                    *dest = strtod(value.data(), &pEnd);

                    return (!pEnd || pEnd == value.data())
							   ? formatOptionalError("Argument", context.name, "float64", value)
                               : none;
			   }}
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, bool* dest)
	: Argument{name, description, [dest](StringView value, Context const&) { return parseBoolean(dest, value); }}
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, StringView* dest)
	: Argument{name, description, [dest](StringView value, Context const&) { *dest = value; return none; }}
{
}


bool
Parser::Argument::isTrailing() const noexcept {
    return name().equals("*");
}


bool
Parser::Option::isMatch(StringView name) const noexcept {
    for (const auto& optName : _names) {
        if (optName == name) {
            return true;
        }
    }

    return false;
}


Optional<Error>
Parser::Option::match(Optional<StringView> const& value, Context const& cntx) const {
    return _callback(value, cntx);
}


Optional<Error>
Parser::Argument::match(StringView const& value, Context const& cntx) const {
    return _callback(value, cntx);
}

