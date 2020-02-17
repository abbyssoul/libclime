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
 * @file: clime/parser.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "clime/parser.hpp"
#include "clime/utils.hpp"

#include <solace/posixErrorDomain.hpp>
#include <solace/output_utils.hpp>

#include <cstring>
#include <cstdlib>


using namespace Solace;
using namespace clime;


const char Parser::DefaultPrefix = '-';
const char Parser::DefaultValueSeparator = '=';



Result<void, Error>
idleAction() noexcept {
    return Ok();
}


Parser::Parser(StringView appDescription)
	: _prefix{DefaultPrefix}
	, _valueSeparator{DefaultValueSeparator}
	, _defaultAction{mv(appDescription), idleAction}
{
}


Parser::Parser(StringView appDescription, std::initializer_list<Option> options)
	: _prefix{DefaultPrefix}
	, _valueSeparator{DefaultValueSeparator}
	, _defaultAction{mv(appDescription), idleAction, options}
{
}


std::pair<StringView, Optional<StringView>>
parseOption(StringView arg, char prefix, char valueSeparator) {
    StringView::size_type const startIndex = (arg.substring(1).startsWith(prefix)) ? 2 : 1;
    if (startIndex >= arg.length()) {
		return std::make_pair(StringView{}, none);
    }

    auto endIndex = startIndex;
    while ((endIndex < arg.length()) && arg[endIndex] != valueSeparator) {
        ++endIndex;
    }

    return (endIndex < arg.length())
               ? std::make_pair(arg.substring(startIndex, endIndex), Optional<StringView>{arg.substring(endIndex + 1)})
               : std::make_pair(arg.substring(startIndex), none);
}


Result<uint32, Error>
parseOptions(Parser::Context const& cntx,
             std::vector<Parser::Option> const& options,
             char prefix, char separator) {
    auto firstPositionalArgument = cntx.offset;

    // Parse array of strings until we error out or there is no more flags:
	for (decltype(firstPositionalArgument) i = firstPositionalArgument;
		 i < cntx.argv.size();
		 ++i, ++firstPositionalArgument) {

		if (!cntx.argv[i]) {
			return Parser::fail(ParserError::InvalidInput, "null string as input");
        }

        auto const arg = StringView{cntx.argv[i]};

        // Check if the arg string starts with a prefix char
        if (!arg.startsWith(prefix)) {  // Nope, not a flag, stop processing options
            break;
        }

        auto [argName, argValue] = parseOption(arg, prefix, separator);
        auto consumeValue = false;

        if (argValue.isNone()) {  // No argument given in --opt=value format, need to examine next argv value
			if (i + 1 < cntx.argv.size()) {  // Check there are more arguments in the argv, so we expect a value
				auto nextArg = StringView{cntx.argv[i + 1]};
                if (!nextArg.startsWith(prefix)) {
                    argValue = std::move(nextArg);
                    consumeValue = true;
                }
            }
        }

        uint32 numberMatched = 0;

        auto const optCntx = cntx.withOffsetAndName(i, argName);

        for (auto& option : options) {
            if (option.isMatch(argName)) {
                if (argValue.isNone() &&
					Parser::ArgumentValue::Required == option.argumentExpectations()) {
                    // Argument is required but none was given, error out!
					// Error message: "Option '{}' expects a value, but none were given", optCntx.name);
					return Parser::fail(ParserError::ValueExpected, "No value given");
                }

                if (consumeValue &&
					Parser::ArgumentValue::NotRequired != option.argumentExpectations()) {
                    consumeValue = false;
                    // Adjust current index in the array
                    ++i;
                    ++firstPositionalArgument;
                }

                numberMatched += 1;

				auto r = option.match((Parser::ArgumentValue::NotRequired == option.argumentExpectations())
									  ? none
									  : argValue,
									  optCntx);
				if (r.isSome()) {
					return r.move();
                }
            }
        }

        if (numberMatched < 1) {
			return Parser::fail(ParserError::UnexpectedValue, "Unexpected option");
        }
    }

    return Ok(firstPositionalArgument);
}


Result<uint32, Error>
parseArguments(Parser::Context const& cntx,
               std::vector<Parser::Argument> const& arguments) {

    bool const expectsTrailingArgument = arguments.empty()
            ? false
            : arguments.back().isTrailing();

    auto const nbPositionalArguments = cntx.argv.size() - cntx.offset;

    if (nbPositionalArguments < arguments.size() && !expectsTrailingArgument) {
		return Parser::fail(ParserError::InvalidNumberOfArgs, "Not enough arguments");
    }

    if (!expectsTrailingArgument && nbPositionalArguments > arguments.size()) {
		return Parser::fail(ParserError::InvalidNumberOfArgs, "Too many arguments");
    }

    auto positionalArgument = cntx.offset;

    // Parse array of strings until we error out or there is no more values to consume:
    for (decltype(positionalArgument) i = 0;
         i < arguments.size() && positionalArgument < cntx.argv.size();
         ++positionalArgument) {

		// Check that we didn't hit argv end:
        if (!cntx.argv[positionalArgument]) {
			return Parser::fail(ParserError::InvalidInput, "null string as input");
        }

        auto& targetArg = arguments[i];
        auto const subCntx = cntx.withOffsetAndName(positionalArgument, targetArg.name());

        auto const arg = StringView {cntx.argv[positionalArgument]};
        auto maybeError = targetArg.match(arg, subCntx);
        if (maybeError) {
			return maybeError.move();
        }

        if (i + 1 < arguments.size()) {
            ++i;
        } else if (!expectsTrailingArgument) {
            ++i;
        }
    }

    if (cntx.argv.size() == positionalArgument) {
        return Ok(positionalArgument);
    }

	return Parser::fail(ParserError::InvalidNumberOfArgs, "Not enough arguments");
}


Result<Parser::ParseResult, Error>
parseCommand(Parser::Command const& cmd, Parser::Context const& cntx) {

    auto optionsParsingResult = parseOptions(cntx,
                                             cmd.options(),
                                             cntx.parser.optionPrefix(),
                                             cntx.parser.valueSeparator());
    if (!optionsParsingResult) {
		return optionsParsingResult.moveError();
    }

    auto const positionalArgument = optionsParsingResult.unwrap();

    // Positional arguments processing
    if (positionalArgument < cntx.argv.size()) {

        if (!cmd.commands().empty()) {
            auto const subcmdName = StringView {cntx.argv[positionalArgument]};
            auto const cmdIt = cmd.commands().find(subcmdName);
            if (cmdIt == cmd.commands().end()) {
				return Parser::fail(ParserError::UnexpectedValue, "Command not supported");
            }

            return parseCommand(cmdIt->second, cntx.withOffsetAndName(positionalArgument + 1, subcmdName));
        } else if (!cmd.arguments().empty()) {
            auto parseResult = parseArguments(cntx.withOffsetAndName(positionalArgument, {}), cmd.arguments());
            if (!parseResult) {
				return parseResult.moveError();
            }

			return Ok(cmd.action());
        } else {
			return Parser::fail(ParserError::UnexpectedValue, "Unexpected arguments given");
        }

    } else {
		auto const& arguments = cmd.arguments();
		if ((arguments.empty() && cmd.commands().empty()) ||
			(!arguments.empty() && arguments.back().isTrailing())) {
			return Ok(cmd.action());
        }

		return Parser::fail(ParserError::InvalidNumberOfArgs, "Not enough arguments");
    }
}


Result<Parser::ParseResult, Error>
Parser::parse(Solace::ArrayView<const char*> args) const {
    if (args.empty()) {
        if (_defaultAction.arguments().empty() && _defaultAction.commands().empty()) {
			return Ok(_defaultAction.action());
        }

		return fail(ParserError::InvalidNumberOfArgs, "Not enough arguments");
    }

    return parseCommand(_defaultAction, {
                            args,
                            1,
                            args[0],
                            *this});
}
