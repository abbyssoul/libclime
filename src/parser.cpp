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


const AtomValue kParserErrorCatergory = atom("cli");


Error clime::makeParserError(ParserError errorCode, StringLiteral tag) noexcept {
    return Error{kParserErrorCatergory, static_cast<int>(errorCode), tag};
}


Result<void, Error>
idleAction() noexcept {
    return Ok();
}


Parser::Parser(StringView appDescription) :
    _prefix(DefaultPrefix),
    _valueSeparator(DefaultValueSeparator),
    _defaultAction(std::move(appDescription), idleAction)
{
}


Parser::Parser(StringView appDescription, std::initializer_list<Option> options) :
    _prefix(DefaultPrefix),
    _valueSeparator(DefaultValueSeparator),
    _defaultAction(std::move(appDescription), idleAction, options)
{
}


std::pair<StringView, Optional<StringView>>
parseOption(StringView arg, char prefix, char valueSeparator) {
    StringView::size_type const startIndex = (arg.substring(1).startsWith(prefix)) ? 2 : 1;
    if (startIndex >= arg.length()) {
        return std::make_pair(StringView(), none);
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
    for (decltype(firstPositionalArgument) i = firstPositionalArgument; i < cntx.argv.size(); ++i, ++firstPositionalArgument) {
        if (!cntx.argv[i]) {
            return Parser::fail("Invalid number of arguments!");
        }

        auto const arg = StringView{cntx.argv[i]};

        // Check if the arg string starts with a prefix char
        if (!arg.startsWith(prefix)) {  // Nope, not a flag, stop processing options
            break;
        }

        // FIXME: C++17 destructuring: auto [argName, argValue] = parseOption(...);
        auto [argName, argValue] = parseOption(arg, prefix, separator);
        auto consumeValue = false;

        if (argValue.isNone()) {  // No argument given in --opt=value format, need to examine next argv value
            if (i + 1 < cntx.argv.size()) {  // Check that there are more arguments in the argv, thus we can expect a value
                StringView nextArg{cntx.argv[i + 1]};

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
                    option.getArgumentExpectations() == Parser::Optionality::Required) {
                    // Argument is required but none was given, error out!
                    return Parser::fail("No value given"); //"Option '{}' expects a value, none were given", optCntx.name);
                }

                if (consumeValue &&
                    option.getArgumentExpectations() != Parser::Optionality::NotRequired) {
                    consumeValue = false;
                    // Adjust current index in the array
                    ++i;
                    ++firstPositionalArgument;
                }

                numberMatched += 1;

                auto r = option.match((option.getArgumentExpectations() == Parser::Optionality::NotRequired)
                                                    ? none
                                                    : argValue,
                                      optCntx);
                if (r.isSome()) {
                    return Err(r.get());
                }
            }
        }

        if (numberMatched < 1) {
            return Parser::fail("Unexpected option"); // '{}'", argName);
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
        return Parser::fail("Not enough arguments");
    }

    if (!expectsTrailingArgument && nbPositionalArguments > arguments.size()) {
        return Parser::fail("Too many arguments");
    }

    auto positionalArgument = cntx.offset;

    // Parse array of strings until we error out or there is no more values to consume:
    for (decltype(positionalArgument) i = 0;
         i < arguments.size() && positionalArgument < cntx.argv.size();
         ++positionalArgument) {

        // Make sure that we didn't hit argv end:
        if (!cntx.argv[positionalArgument]) {
            return Parser::fail("Invalid number of arguments!");
        }

        auto& targetArg = arguments[i];
        auto const subCntx = cntx.withOffsetAndName(positionalArgument, targetArg.name());

        auto const arg = StringView {cntx.argv[positionalArgument]};
        auto maybeError = targetArg.match(arg, subCntx);
        if (maybeError) {
            return Err(maybeError.move());
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

    return Parser::fail("Not enough arguments");
}


Result<Parser::ParseResult, Error>
parseCommand(Parser::Command const& cmd, Parser::Context const& cntx) {

    auto optionsParsingResult = parseOptions(cntx,
                                             cmd.options(),
                                             cntx.parser.optionPrefix(),
                                             cntx.parser.valueSeparator());
    if (!optionsParsingResult) {
        return Err(optionsParsingResult.moveError());
    }

    auto const positionalArgument = optionsParsingResult.unwrap();

    // Positional arguments processing
    if (positionalArgument < cntx.argv.size()) {

        if (!cmd.commands().empty()) {
            auto const subcmdName = StringView {cntx.argv[positionalArgument]};
            auto const cmdIt = cmd.commands().find(subcmdName);
            if (cmdIt == cmd.commands().end()) {
                return Parser::fail("Command not supported"); //, subcmdName);
            }

            return parseCommand(cmdIt->second, cntx.withOffsetAndName(positionalArgument + 1, subcmdName));
        } else if (!cmd.arguments().empty()) {
            auto parseResult = parseArguments(cntx.withOffsetAndName(positionalArgument, {}), cmd.arguments());
            if (!parseResult) {
                return Err(parseResult.moveError());
            }

            return Ok<Parser::ParseResult>(cmd.action());
        } else {
            return Parser::fail("Unexpected arguments given");
        }

    } else {
        if ((cmd.arguments().empty() && cmd.commands().empty()) ||
            (!cmd.arguments().empty() && cmd.arguments().back().isTrailing())) {
            return  Ok<Parser::ParseResult>(cmd.action());
        }

        return Parser::fail("Not enough arguments");
    }
}


Result<Parser::ParseResult, Error>
Parser::parse(Solace::ArrayView<const char*> args) const {
    if (args.empty()) {
        if (_defaultAction.arguments().empty() && _defaultAction.commands().empty()) {
            return Ok<Parser::ParseResult>(_defaultAction.action());
        }

        return fail("Not enough arguments");
    }

    return parseCommand(_defaultAction, {
                            args,
                            1,
                            args[0],
                            *this});
}
