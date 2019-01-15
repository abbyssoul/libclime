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
 * @file: clime/helpPrinter.cpp
 *
*******************************************************************************/

#include "clime/parser.hpp"
#include "clime/utils.hpp"
#include "clime/parseUtils.hpp"

//#include <solace/path.hpp>
#include <solace/posixErrorDomain.hpp>
#include <solace/output_utils.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>  // std::stringstream

#include <utility>


using namespace Solace;
using namespace clime;


namespace {
void formatOption(std::ostream& output, char prefixChar, Parser::Option const& option) {
    std::stringstream s;
    s << "  ";

    bool chained = false;
    for (auto const& optName : option.names()) {
        if (chained) {
            s << ", ";
        }

        if (optName.length() == 1) {
            s << prefixChar << optName;
        } else {
            s << prefixChar << prefixChar << optName;
        }

        chained = true;
    }

    output << std::left << std::setw(26) << s.str() << option.description() << std::endl;
}


void formatCommand(std::ostream& output, StringView name, Parser::Command const& cmd) {
    output << "  " << std::left << std::setw(14)
            << name << cmd.description() << std::endl;
}

}  // namespace


void
HelpFormatter::operator() (std::ostream& output,
                           StringView name,
                           Parser::Command const& cmd
                           ) {
    output << "Usage: " << name;  // Path::parse(c.argv[0]).getBasename();

    if (!cmd.options().empty()) {
        output << " [options]";
    }

    if (!cmd.arguments().empty()) {
        for (auto const& arg : cmd.arguments()) {
            output << " [" << arg.name() <<"]";
        }
    }


    if (!cmd.commands().empty()) {
        output << " <command>";
    }

    output << std::endl;
    output << cmd.description() << std::endl;

    if (!cmd.options().empty()) {
        output << "Options:" << std::endl;

        for (auto const& opt : cmd.options()) {
            formatOption(output, _optionsPrefix, opt);
        }
    }

    if (!cmd.commands().empty()) {
        output << "Commands:" << std::endl;

        for (auto const& subcmd : cmd.commands()) {
            formatCommand(output, subcmd.first, subcmd.second);
        }
    }
}



void
VersionPrinter::operator() (std::ostream& output) {
    output << _canonicalAppName << " " << _version << std::endl;
}



Parser::Option
Parser::printVersion(StringView appName, Version const& appVersion) {
    return {{"v", "version"}, "Print version", Parser::Optionality::NotRequired,
            [appName, &appVersion] (Optional<StringView> const&, Context const&) -> Optional<Error> {
                VersionPrinter(appName, appVersion)
                    (std::cout);

                return none;
            }};
}


Parser::Option
Parser::Parser::printHelp() {
    return {{"h", "help"}, "Print help", Parser::Optionality::NotRequired,
            [](Optional<StringView> const& value, Context const& cntx) -> Optional<Error> {
                HelpFormatter printer(cntx.parser.optionPrefix());

                if (value.isNone()) {
                    printer(std::cout,
                            cntx.argv[0],
                            cntx.parser.defaultAction());
                } else {
                    auto const& cmdIt = cntx.parser.commands().find(value.get());
                    if (cmdIt == cntx.parser.commands().end()) {
                        return Optional<Error>(makeError(BasicError::InvalidInput, "help"));
                    }

                    printer(std::cout,
                            cmdIt->first,
                            cmdIt->second);
                }

                return none;
            }};
}


Parser::Command::CommandDict::value_type
Parser::printVersionCmd(StringView appName, Version const& appVersion) {
    return {"version", {
                "Print version",
                [appName, &appVersion]() -> Result<void, Error> {
                    VersionPrinter(appName, appVersion)(std::cout);
                    return Ok();
                }
               }};
}


Parser::Command::CommandDict::value_type
Parser::printHelpCmd() {
    return {"help", {
            "Print help",
            []() -> Result<void, Error> {
                HelpFormatter printer(Parser::DefaultPrefix);
//                printer(std::cout);

                return Ok();
            }
        }};
}
