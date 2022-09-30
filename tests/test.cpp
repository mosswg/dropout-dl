//
// Created by moss on 9/30/22.
//
#include "test.h"

template<typename t>
void dropout_dl::test<t>::display_result() {
    if (!this->success) {
        std::cout << RED << name << ": \"" << result << "\" =/= \"" << expected_result << '"' << RESET << std::endl;
    } else {
        std::cout << GREEN << name << RESET << std::endl;
    }
}

void dropout_dl::tests::display() {

    if (!this->success) {
        std::cout << '\n' << TESTNAME <<  BOLDRED << name << RESET << std::endl;
    } else {
        std::cout << '\n' << TESTNAME << BOLDGREEN << name << RESET << std::endl;
    }

    for (auto& test : tests_vector) {
        test.display_result();
    }
}