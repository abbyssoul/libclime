/*
*  Copyright 2020 Ivan Ryabov
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
 * libclime Unit Test Suit
 * @file: test/extras/test_multivalueParser.cpp
 * @author: abbyssoul
*******************************************************************************/
#include <clime/extras/multivalueParser.hpp>

#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace clime;


TEST(TestMultivalueParser, emptyParserHasNoValues) {
	auto parser = extras::MultivalueParser<DialString, decltype(&tryParseDailString)>{&tryParseDailString};
	EXPECT_FALSE(parser.hasValues());
}

TEST(TestMultivalueParser, parseSingleValue) {
	AtomValue expectedProtocols[] = { kProtocolTCP, kProtocolUnix, kProtocolTCP };

	auto parser = extras::MultivalueParser<DialString, decltype(&tryParseDailString)>{&tryParseDailString};
	auto result = parser.parseOption("tcp:localhost:1,unix:localhost:2,tcp:localhost:3");
	EXPECT_TRUE(result.isOk());

	auto& values = *result;
	EXPECT_EQ(3, values.size());

	for (Parser::Context::size_type i = 0; i < 3; ++i) {
		EXPECT_EQ(expectedProtocols[i], values[i].protocol);
	}
}


TEST(TestMultivalueParser, parseList) {
	auto parser = extras::MultivalueParser<DialString, decltype(&tryParseDailString)>{&tryParseDailString};

	AtomValue expectedProtocols[] = { kProtocolTCP, kProtocolUDP, kProtocolUnix, kProtocolTCP };
	const char* argv[] = { "tcp:localhost:1,udp:10.0.0.3:123", "unix:localhost:2", "tcp:localhost:3"};

	auto cliParser = Parser{"test_app"};
	auto context = Parser::Context{ arrayView(argv), 0, "option", cliParser };

	for (Parser::Context::size_type i = 0; i < context.argv.size(); ++i) {
		context.offset = i;

		auto maybeError = parser(argv[i], context);
		ASSERT_TRUE(maybeError.isNone());
	}

	EXPECT_TRUE(parser.hasValues());
	EXPECT_EQ(4, parser.values.size());

	for (Parser::Context::size_type i = 0; i < 4; ++i) {
		EXPECT_EQ(expectedProtocols[i], parser.values[i].protocol);
	}
}
