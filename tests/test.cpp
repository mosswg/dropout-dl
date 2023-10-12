//
// Created by moss on 9/30/22.
//
#include "test.h"
#include "series_tests.h"
#include "season_tests.h"
#include "episode_tests.h"

template<typename t>
void dropout_dl::test<t>::display_result() {
    std::cout << "\t\t";

    if (!this->success) {
        std::cout << RED << name << ": \"" << result << "\" =/= \"" << expected_result << '"' << RESET << std::endl;
    } else {
        std::cout << GREEN << name << RESET << std::endl;
    }
}

void dropout_dl::tests::display() {

    int success_count = 0;

    for (auto& test : this->tests_vector) {
        success_count += test.success;
    }

    std::cout << '\t' << TESTNAME;

    if (!this->success) {
        std::cout << BOLDRED;
    } else {
        std::cout << BOLDGREEN;
    }

    std::cout  << name << " [" << success_count << "/" << this->tests_vector.size() << "]" << RESET << std::endl;

    for (auto& test : tests_vector) {
        test.display_result();
    }
}


int main() {
    std::string episode_tests_name = "Episode Tests";
    std::vector<dropout_dl::tests> episode_tests = test_episode();

    bool episode_tests_success = true;
    int episode_success_count = 0;


    for (const auto& test : episode_tests) {
        episode_tests_success &= test.success;
        episode_success_count += test.success;
    }

    if (episode_tests_success) {
        std::cout << TESTNAME << BOLDGREEN << episode_tests_name;
    }
    else {
        std::cout << TESTNAME << BOLDRED << episode_tests_name;
    }

    std::cout << " [" << episode_success_count << "/" << episode_tests.size() << "]" << RESET << std::endl;

    for (auto& test : episode_tests) {
        test.display();
    }


    std::cout << "\n";


    std::string series_tests_name = "Series Tests";
    std::vector<dropout_dl::tests> series_tests = test_series();

    bool series_tests_success = true;
    int series_success_count = 0;
    for (const auto& test : series_tests) {
        series_tests_success &= test.success;
        series_success_count += test.success;
    }

    if (series_tests_success) {
        std::cout << TESTNAME << BOLDGREEN << series_tests_name;
    }
    else {
        std::cout << TESTNAME << BOLDRED << series_tests_name;
    }


    std::cout << " [" << series_success_count << "/" << series_tests.size() << "]" << RESET << std::endl;

    for (auto& test : series_tests) {
        test.display();
    }


    std::string season_tests_name = "Season Tests";
    std::vector<dropout_dl::tests> season_tests = test_season();

    bool season_tests_success = true;
    int season_success_count = 0;
    for (const auto& test : season_tests) {
        season_tests_success &= test.success;
        season_success_count += test.success;
    }

    if (season_tests_success) {
        std::cout << TESTNAME << BOLDGREEN << season_tests_name;
    }
    else {
        std::cout << TESTNAME << BOLDRED << season_tests_name;
    }


    std::cout << " [" << season_success_count << "/" << season_tests.size() << "]" << RESET << std::endl;

    for (auto& test : season_tests) {
        test.display();
    }
}
