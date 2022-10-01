//
// Created by moss on 9/30/22.
//
#include "test.h"
#include "series_tests.h"
#include "episode_tests.h"

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


int main() {
    std::string episode_tests_name = "Episode Tests";
    std::vector<dropout_dl::tests> episode_tests = test_episode();

    bool episode_tests_success = true;
    for (const auto& test : episode_tests) {
        episode_tests_success &= test.success;
    }

    if (episode_tests_success) {
        std::cout << TESTNAME << BOLDGREEN << episode_tests_name << std::endl;
    }
    else {
        std::cout << TESTNAME << BOLDRED << episode_tests_name << std::endl;
    }

    for (auto& test : episode_tests) {
        test.display();
    }


    std::cout << "\n\n";


    std::string series_tests_name = "Series Tests";
    std::vector<dropout_dl::tests> series_tests = test_series();

    bool series_tests_success = true;
    for (const auto& test : episode_tests) {
        series_tests_success &= test.success;
    }

    if (series_tests_success) {
        std::cout << TESTNAME << BOLDGREEN << series_tests_name << std::endl;
    }
    else {
        std::cout << TESTNAME << BOLDRED << series_tests_name << std::endl;
    }

    for (auto& test : series_tests) {
        test.display();
    }
}