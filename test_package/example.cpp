#include <clime/parser.hpp>
#include <solace/output_utils.hpp>

#include <iostream>  // std::cerr
#include <cstdlib>  // EXIT_SUCCESS/EXIT_FAILURE

using namespace clime;

int main(int argc, const char **argv) {
	auto const res = Parser("clime cli single action example", {
                Parser::printHelp(),
                Parser::printVersion("Package test", Solace::Version(1, 0, 1)),
            })
            .parse(argc, argv);

    if (!res) {
        std::cerr << res.getError();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
