//
// Created by moss on 9/30/22.
//
#pragma once

#include <vector>
#include "iostream"

namespace dropout_dl {

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define FAIL     "\033[31mFAIL: \033[0m"             // Test Failure
#define BOLDFAIL     "\033[1m\033[31mFAIL: \033[0m"             // Test Failure
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define SUCCESS   "\033[32mSUCCESS: \033[0m"         /* Test Success */
#define BOLDSUCCESS   "\033[1m\033[32mSUCCESS: \033[0m"         /* Test Success */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define WARN      "\033[1m\033[33mWARNING: \033[0m"         /* Test Warning */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define TESTNAME    "\033[1m\033[34mTEST: \033[0m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


    template <typename t> class test {
        public:
            std::string name;
            t result;
            t expected_result;
            bool success;

            test(const std::string& name, const t& result, const t& expected_result) {
                this->name = name;
                this->result = result;
                this->expected_result = expected_result;
                this->success = (result == expected_result);
            }

            test(const std::string& test_name, t (*function)(const t &), const t &argument, const t& expected_result) {
                t test_result = function(argument);
                this->name = test_name;
                this->result = test_result;
                this->expected_result = expected_result;
                this->success = test_result == expected_result;
            }

            void display_result();


    };


    class tests {
        public:
            std::vector<test<std::string>> tests_vector;
            std::string name;
            bool success;

            tests(const std::string& name, const std::vector<test<std::string>>& tests) {
                this->name = name;
                this->tests_vector = tests;
                this->success = tests_vector.front().success;
                for (const auto& test : tests_vector) {
                    success = success && test.success;
                }
            }

            void display();
    };


}