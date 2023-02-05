//
// Created by moss on 9/30/22.
//
#pragma once

#include <vector>
#include "iostream"

#include "../src/color.h"

namespace dropout_dl {

    template <typename t> class test {
        public:
            std::string name;
            t result;
            t expected_result;
            bool success = false;

            test(std::string  name, const t& result, const t& expected_result) : name(std::move(name)), result(result), expected_result(expected_result), success(result == expected_result) {}

            test(std::string  test_name, t (*function)(const t &), const t &argument, const t& expected_result) : name(std::move(test_name)), expected_result(expected_result) {
                t test_result = function(argument);
                this->result = test_result;
                this->success = test_result == expected_result;
            }

            void display_result();


    };


    class tests {
        public:
            std::vector<test<std::string>> tests_vector;
            std::string name;
            bool success;

            tests(std::string name, std::vector<test<std::string>> tests) : name(std::move(name)), tests_vector(std::move(tests)) {
                this->success = tests_vector.front().success;
                for (const auto& test : tests_vector) {
                    success = success && test.success;
                }
            }

            void display();
    };


}
