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

/**
 * An example of command line argument parser for a multi-action CLI.
*/

#include <clime/parser.hpp>
#include <solace/output_utils.hpp>

#include <iostream>


using namespace Solace;
using namespace clime;


static constexpr StringLiteral      kAppName = "cli_single";
static const Version                kAppVersion = Version(0, 0, 1, "dev");


static uint intValue = 3;
static float32 floatValue = 0.0f;
static StringView userName{getenv("USER")};


Result<void, Error> sayHi() {
	std::cout << "Hello '" << userName << "'" << '\n';

    return Ok();
}

Result<void, Error> list() {
    for (uint i = 0; i < intValue; ++i) {
		std::cout << " - " << i << '\n';
    }

    return Ok();
}


static int addArg_1 = 0;
static int addArg_2 = 0;

Result<void, Error>
addNumbers() {
	std::cout << addArg_1 << " + " << addArg_2 << " = " << addArg_1 + addArg_2 << '\n';

    return Ok();
}


int main(int argc, const char **argv) {

    auto res = Parser("Solace cli multi action example", {
                Parser::printHelp(),
                Parser::printVersion(kAppName, kAppVersion),

				{{"i", "listCounter"},	"Listing size", &intValue},
				{{"fOption"},			"Foating point value for the demo", &floatValue},
				{{"u", "name"},			"Greet user name", &userName}
            })
            .commands({
						  {"greet-1", {"Say Hi to the user", sayHi}},
                          {"count", {"Print n numbers", list}},
						  {"add",	{"Add numbers", {
                                       {"arg1", "1st argument", &addArg_1},
                                       {"arg2", "2nd argument", &addArg_2}
                                   },
                                   addNumbers}}
            })
            .parse(argc, argv);

	if (!res) {
		auto& error = res.getError();
		if (error) {
			std::cerr << res.getError().toString() << '\n';
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	res.unwrap()();  // Do the selected action.
	return EXIT_SUCCESS;
}
