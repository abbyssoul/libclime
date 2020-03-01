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
 * libclime: Command line arguments parser
 *	@file		clime/extras/multivalueParser.hpp
 *	@brief		Parser for multivalue option.
 ******************************************************************************/
#pragma once
#ifndef CLIME_EXTRAS_MULTIVALUEPARSER_HPP
#define CLIME_EXTRAS_MULTIVALUEPARSER_HPP

#include "clime/parser.hpp"


namespace clime::extras {


template<typename ValueType,  // type of the value
		 typename F>		  // Value parser functor
struct MultivalueParser {
	using Value = ValueType;
	using ValueSet = std::vector<Value>;

	template<typename F_>
	MultivalueParser(F_&& parser) noexcept
		: _parseValue{Solace::mv(parser)}
	{}

	auto parseOption(Solace::StringView value) {
		using namespace Solace;
		using size_type = StringView::size_type;
		Result<std::vector<Value>, Error> result{types::okTag, in_place};

		auto& values = *result;
		value.split(",", [&result, &values, this](StringView split, size_type i, size_type count) {
			if (!result)
				return;

			auto rest = _parseValue(split);
			if (!rest) {
				result = rest.moveError();
				return;
			}

			if (i == 0 && count != 0 && values.empty()) {
				values.reserve(count);
			}

			values.emplace_back(*rest);
		});

		return result;
	}

	Solace::Optional<Error>
	operator() (Solace::Optional<Solace::StringView> const& value, clime::Parser::Context const& /*cntx*/) {
		auto res = parseOption(*value);
		if (!res) {
			return res.moveError();
		}

		auto& parsedValues = res.unwrap();
		values.reserve(values.size() + parsedValues.size());
		for (auto& ds : parsedValues) {
			values.emplace_back(ds);
		}

		// No errors encontered
		return Solace::none;
	}

	bool hasValues() const noexcept {
		return !values.empty();
	}


	ValueSet values;

private:
	F	_parseValue;
};


}  // End of namespace clime::extras
#endif  // CLIME_EXTRAS_MULTIVALUEPARSER_HPP
