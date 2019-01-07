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
 * libclime: Command line parser
 *	@file		clime/parser.hpp
 *	@brief		Commandline arguments Parser
 ******************************************************************************/
#pragma once
#ifndef CLIME_PARSER_HPP
#define CLIME_PARSER_HPP

#include <solace/stringView.hpp>
#include <solace/result.hpp>
#include <solace/error.hpp>
#include <solace/array.hpp>
#include <solace/vector.hpp>
#include <solace/version.hpp>

// TODO(abbyssoul): consider moving away from std::function #include "solace/delegate.hpp"
#include <solace/utils.hpp>

// Note unordered map has issues with class not being fully defined.
#include <map>      // TODO(abbyssoul): Replace with fix-memory map
#include <vector>   // TODO(abbyssoul): Replace with fix-memory vector
#include <functional>   // TODO(abbyssoul): Replace with a better delegate, maybe?


namespace clime {

/**
 * Command line parser
 * This is a helper class to handle processing of command line arguments.
 *
 * @example
 * \code{.cpp}
 int main(int argc, argv) {

  cli::Parser("My application", {
                // Custom handler example:
                CommandlineParser::printVersion("my_app", Version(1, 2, 3, "dev")),
                CommandlineParser::printHelp(),

                // Regular options of integral type
                {{"size"}, "Buffer size", &settings.bufferSize },
                {{"u", "userName"}, "User name", &settings.userName }
            })
            .commands({
                    {"doSomething", { // Commands support optional arguments:
                        { "Mandatory argument", &settings.param },
                        [] () {   // Action to execute.
                            std::cout << "Executing command" << std:endl;
                        }
                    }
            }})
            .parse(argc, argv)
            .then(...consume parsing results...
            .orElse(...handle errors...);
    ...

    \endcode


 * Note that command line parsing is meant to be performed once at the start of an application.
 * Keeping in mind that memory allocation limits and allocation strategy is yet to be configured,
 * it is desirable that parser don't allocate any memory for parsing.
 *
 * To support that, a parser implementation is using StringView and StringLiteral types,
 * that don't require ownership/allocation of a string buffer.
 */
class Parser {
public:

    /**
     * Parser context.
     * This object represents the current state of parsing.
     * It is designed to be used by a callback function to get access to the parameters and parser object itself.
     * It also can be used to communicate back to the parser if an interruption is required.
     */
    struct Context {

        /// Initial number of arguments passed to the 'parse' method.
        const Solace::uint32 argc;

        /// Individual command line arguments the parse method has been given.
        const char** argv;

        /// Current parser offset into argv.
        const Solace::uint32 offset;

        /// Name of the option / argument being parsed.
        const Solace::StringView name;

        /// Reference to the instance of the parser that invokes the callback.
        Parser const& parser;

        Context(Solace::uint32 inArgc, char const* inArgv[], Solace::uint32 inOffset,
                Solace::StringView inName,
                Parser const& self) :
            argc(inArgc),
            argv(inArgv),
            offset(inOffset),
            name(inName),
            parser(self)
        {}

    };

    /**
     * Argument processing policy for custom callbacks
     */
    enum class OptionArgument {
        Required,          //!< Argument is required. It is an error if the option is given without an value.
        Optional,          //!< Argument is optional. It is not an error to have option with or without an argument.
        NotRequired        //!< Argument is not expected. It is an error to give an option with an argument value.
    };


    /**
     * An optional argument / flag object used by command line parser.
     */
    class Option {
    public:
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::StringView* val);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::int8* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::uint8* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::int16* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::uint16* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::int32* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::uint32* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::int64* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::uint64* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::float32* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, Solace::float64* value);
        Option(std::initializer_list<Solace::StringLiteral> names, Solace::StringLiteral desc, bool* value);

        /// Common constructor:
        template<typename F>
        Option(std::initializer_list<Solace::StringLiteral> names,
               Solace::StringLiteral description,
               OptionArgument expectsArgument,
               F&& f) :
            _names(names),
            _description(std::move(description)),
            _expectsArgument(expectsArgument),
            _callback(std::forward<F>(f))
        {}

        Option(Option const& rhs) = default;

        Option(Option&& rhs) noexcept = default;

        Option& operator= (Option const& rhs) noexcept {
            Option(rhs).swap(*this);

            return *this;
        }

        Option& operator= (Option&& rhs) noexcept {
            return swap(rhs);
        }

        Option& swap(Option& rhs) noexcept {
            using std::swap;
            swap(_names, rhs._names);
            swap(_description, rhs._description);
            swap(_callback, rhs._callback);
            swap(_expectsArgument, rhs._expectsArgument);

            return (*this);
        }

        bool isMatch(Solace::StringView argName) const noexcept;

        Solace::Optional<Solace::Error>
        match(Solace::Optional<Solace::StringView> const& value, Context const& c) const;

        const std::vector<Solace::StringLiteral>& names() const noexcept    { return _names; }
        const Solace::StringLiteral& description() const noexcept           { return _description; }

        OptionArgument getArgumentExpectations() const noexcept     { return _expectsArgument; }

    private:
        //!< Long name of the option, Maybe empty if not specified.
        std::vector<Solace::StringLiteral>          _names;

        //!< Human-readable description of the option.
        Solace::StringLiteral                       _description;

        //!< Enum to indicate if this option expects a value or not.
        OptionArgument                      _expectsArgument;

        //!< A callback to be called when this option is encountered in the input cmd line.
        std::function<Solace::Optional<Solace::Error> (Solace::Optional<Solace::StringView> const&, Context const&)>
        _callback;
    };


    /** Mandatory argument
     * This class represent a mandatory argument to be expected by a parser.
     * It is a parsing error if no mandatory arguments is provided.
     */
    class Argument {
    public:
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::StringView* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::int8* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::uint8* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::int16* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::uint16* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::int32* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::uint32* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::int64* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::uint64* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::float32* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, Solace::float64* value);
        Argument(Solace::StringLiteral name, Solace::StringLiteral description, bool* value);

        template<typename F>
        Argument(Solace::StringLiteral name, Solace::StringLiteral description,
                 F&& callback) :
            _name(std::move(name)),
            _description(std::move(description)),
            _callback(std::forward<F>(callback))
        {}

        Argument(Argument const& rhs) = default;

        Argument(Argument&& rhs) noexcept = default;

        Argument& operator= (Argument const& rhs) noexcept {
            Argument(rhs).swap(*this);

            return *this;
        }

        Argument& operator= (Argument&& rhs) noexcept {
            return swap(rhs);
        }

        Argument& swap(Argument& rhs) noexcept {
            std::swap(_name, rhs._name);
            std::swap(_description, rhs._description);
            std::swap(_callback, rhs._callback);

            return (*this);
        }

        Solace::StringView name() const noexcept {
            return _name;
        }

        bool isTrailing() const noexcept;

        Solace::Optional<Solace::Error> match(Solace::StringView const& value, Context const& c) const;

    private:
        Solace::StringLiteral                               _name;
        Solace::StringLiteral                               _description;
        std::function<Solace::Optional<Solace::Error> (Solace::StringView, Context const&)>    _callback;
    };


    /**
     * Command for CLI
     */
    class Command {
    public:

        using CommandDict = std::map<Solace::StringView, Command>;
        using Action = std::function<Solace::Result<void, Solace::Error>()>;

        ~Command() = default;

        Command(Command const& rhs) = default;

        Command(Command&& rhs) = default;

        template<typename F>
        Command(Solace::StringView description, F&& f) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options()
        {}

        template<typename F>
        Command(Solace::StringView description,
                F&& f,
                std::initializer_list<Option> options) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(options),
            _commands(),
            _arguments()
        {}

        template<typename F>
        Command(Solace::StringView description,
                std::initializer_list<Argument> arguments,
                F&& f) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(),
            _commands(),
            _arguments(arguments)
        {}

        template<typename F>
        Command(Solace::StringView description,
                std::initializer_list<Argument> arguments,
                F&& f,
                std::initializer_list<Option> options) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(options),
            _commands(),
            _arguments(arguments)
        {}

        Command& operator= (Command const& rhs) = default;
        Command& operator= (Command&& rhs) noexcept = default;

        Command& swap(Command& rhs) noexcept {
            std::swap(_description, rhs._description);
            std::swap(_callback, rhs._callback);
            std::swap(_options, rhs._options);
            std::swap(_commands, rhs._commands);
            std::swap(_arguments, rhs._arguments);

            return (*this);
        }

        Solace::StringView description() const noexcept           { return _description; }
        Command& description(Solace::StringView description) noexcept {
            _description = description;
            return *this;
        }

        const std::vector<Option>& options() const noexcept               { return _options; }
        Command& options(std::initializer_list<Option> options) {
            _options = options;
            return *this;
        }

        const CommandDict&  commands() const noexcept  { return _commands; }
//        Command& commands(std::initializer_list<CommandDict::value_type> commands) {
        Command& commands(std::initializer_list<std::pair<Solace::StringView const, Command>> commands) {
            _commands = commands;
            return *this;
        }

        const std::vector<Argument>& arguments() const noexcept           { return _arguments; }
        Command& arguments(std::initializer_list<Argument> arguments) {
            _arguments = arguments;
            return *this;
        }

        Action action() const {
            return _callback;
        }

        template<typename F>
        Command& action(F&& f) {
            _callback = std::forward<F>(f);
            return *this;
        }


    private:
        Solace::StringView      _description;
        Action                  _callback;

        /// Options / flags that the command accepts.
        std::vector<Option>   _options;

        /// Sub-commands
        CommandDict     _commands;

        /// Mandatory positional arguments
        std::vector<Argument> _arguments;
    };


public:

    //!< Default prefix for flags and options
    static const char DefaultPrefix;

    //!< Default value separator
    static const char DefaultValueSeparator;

public:

    ~Parser() = default;

    /// Default copy-constructor
    Parser(Parser const& rhs) = default;

    /// Default move-constructor
    Parser(Parser&& rhs) noexcept = default;

    /**
     * Construct default command line parser.
     *
     * @param appDescription Human readable application description to be used by 'help'-type commands.
     */
    Parser(Solace::StringView appDescription);

    /**
     * Construct a commandline parser with application description and a list of expected options.
     * @param appDescription Hyman readable application description to be used by 'help'-type commands.
     * @param options Initializer-list of command line options. @see CommandlineParser::options() for more info.
     */
    Parser(Solace::StringView appDescription, std::initializer_list<Option> options);

    Parser& operator= (Parser const& rhs) noexcept {
        Parser(rhs).swap(*this);

        return *this;
    }

    Parser& operator= (Parser&& rhs) noexcept {
        return swap(rhs);
    }

    Parser& swap(Parser& rhs) noexcept {
        using std::swap;
        swap(_prefix, rhs._prefix);
        swap(_valueSeparator, rhs._valueSeparator);
        swap(_defaultAction, rhs._defaultAction);

        return (*this);
    }


    using ParseResult = std::function<Solace::Result<void, Solace::Error>()>;

    /**
     * Parse command line arguments and process all the flags.
     * @param argc Number of command line arguments including name of the program at argv[0]
     * @param argv An array of string that represent command line argument tokens.
     * @return Result of parsing: Either a pointer to the parser or an error.
     */
    Solace::Result<ParseResult, Solace::Error>
    parse(int argc, const char* argv[]) const;


    /**
     * Add an option to print application version.
     * @param appName Name of the application to print along with version.
     * @param appVersion Application version to be printed.
     * @return A parser option that when given by a user will result in a printing of the version info.
     */
    static Option printVersion(Solace::StringView appName, Solace::Version const& appVersion);


    /**
     * Add a command to print application's version.
     * @param appName Name of the application to print along with version.
     * @param appVersion Application version to be printed.
     * @return A parser command that when given by a user will result in a printing of the version info.
     */
    static Command::CommandDict::value_type
    printVersionCmd(Solace::StringView appName, Solace::Version const& appVersion);


    /**
     * Add an option to print help summary.
     * @return A parser option that when given by a user will result in a printing of
     * short options summary.
     */
    static Option printHelp();

    /**
     * Add a command to print help summary.
     * @return A parser command that when given by a user will result in a printing of help summary.
     */
    static Command::CommandDict::value_type printHelpCmd();


    /**
     * Get prefix used to identify flags and options.
     * @return prefix for flags and options.
     */
    char optionPrefix() const noexcept { return _prefix; }

    /**
     * Set prefix used to identify flags and options.
     * @param prefixChar A new value for the prefix character.
     * @return Reference to this for fluent interface.
     */
    Parser& optionPrefix(char prefixChar) noexcept {
        _prefix = prefixChar;
        return *this;
    }

    /**
     * Get prefix used to identify flags and options.
     * @return prefix for flags and options.
     */
    char valueSeparator() const noexcept { return _valueSeparator; }

    /**
     * Set prefix used to identify flags and options.
     * @param prefixChar A new value for the prefix character.
     * @return Reference to this for fluent interface.
     */
    Parser& valueSeparator(char value) noexcept {
        _valueSeparator = value;
        return *this;
    }


    /**
     * Get human readable description of the application, dispayed by help and version commands.
     * @return Human readable application description string.
     */
    Solace::StringView description() const noexcept { return _defaultAction.description(); }

    /**
     * Set human-readable application description, to be dispayed by 'help' and 'version' commands.
     * @param desc New value for human-readable application description string.
     * @return Reference to this for fluent interface.
     */
    Parser& description(Solace::StringView desc) noexcept {
        _defaultAction.description(desc);

        return *this;
    }

    const std::vector<Option>& options() const noexcept       { return _defaultAction.options(); }
    Parser& options(std::initializer_list<Option> options) {
        _defaultAction.options(options);

        return *this;
    }

    Command::CommandDict const& commands() const noexcept        { return _defaultAction.commands(); }
    Parser& commands(std::initializer_list<Command::CommandDict::value_type> commands) {
        _defaultAction.commands(commands);

        return *this;
    }

    const std::vector<Argument>& arguments() const noexcept       { return _defaultAction.arguments(); }
    Parser& arguments(std::initializer_list<Argument> arguments) {
        _defaultAction.arguments(arguments);

        return *this;
    }

    Command&        defaultAction() noexcept        { return _defaultAction; }
    Command const&  defaultAction() const noexcept  { return _defaultAction; }

    template<typename F>
    Command const& defaultAction(F&& f) {
        _defaultAction.action(std::forward<F>(f));

        return _defaultAction;
    }

private:

    /// Option prefix
    char            _prefix;

    /// Value separator
    char            _valueSeparator;

    /// Default action to be produced when no other commands specified.
    Command         _defaultAction;
};



inline void swap(Parser::Option& lhs, Parser::Option& rhs) noexcept { lhs.swap(rhs); }

inline void swap(Parser::Argument& lhs, Parser::Argument& rhs) noexcept { lhs.swap(rhs); }

inline void swap(Parser::Command& lhs, Parser::Command& rhs) noexcept { lhs.swap(rhs); }

inline void swap(Parser& lhs, Parser& rhs) noexcept { lhs.swap(rhs); }

}  // End of namespace clime
#endif  // CLIME_PARSER_HPP
