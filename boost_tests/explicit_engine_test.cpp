#define BOOST_TEST_MODULE explicit_engine

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <set>

#include "utils.h"
#include "PetriEngine/ExplicitColored/ExplicitColoredModelChecker.h"

using namespace PetriEngine;
using namespace PetriEngine::ExplicitColored;
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(DirectoryTest) {
    BOOST_REQUIRE(getenv("TEST_FILES"));
}

void test_explicit_engine(const char* fn, ExplicitColoredModelChecker::Result expected, size_t quid = 0,
                          uint32_t kbound = 4, const char* expectedOutput = nullptr) {
    std::string model = std::string("/models/explicit-engine/") + fn + ".pnml";
    std::string query = std::string("/models/explicit-engine/") + fn + ".xml";
    std::set<size_t> qnums{quid};
    auto [queries, querynames, sset, options] = load_explicit(model, query, qnums);
    options.kbound = kbound;

    ExplicitColoredModelChecker checker(sset, std::cout);
    std::ostringstream output;
    ColoredResultPrinter printer(quid, output, querynames[0], options.seed(), output);
    auto result = checker.checkQuery(queries[0], options, expectedOutput ? &printer : nullptr);
    
    BOOST_REQUIRE_EQUAL(expected, result);
    if (expectedOutput) {
        BOOST_REQUIRE(output.str().find(expectedOutput) != std::string::npos);
    }
}

BOOST_AUTO_TEST_CASE(SubtractionWithVars, * utf::timeout(5)) {
    test_explicit_engine("subtraction_with_vars", ExplicitColoredModelChecker::Result::SATISFIED);
}

BOOST_AUTO_TEST_CASE(ReferendumColoredSubtraction, * utf::timeout(5)) {
    test_explicit_engine("referendum_colored_subtraction", ExplicitColoredModelChecker::Result::SATISFIED);
}

BOOST_AUTO_TEST_CASE(KBound, * utf::timeout(5)) {
    test_explicit_engine("referendum_colored_subtraction", ExplicitColoredModelChecker::Result::UNSATISFIED, 1, 9,
                         "FORMULA Ten voting tokens are reachable FALSE");
    test_explicit_engine("referendum_colored_subtraction", ExplicitColoredModelChecker::Result::SATISFIED, 1, 10);
}
