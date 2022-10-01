//
// Created by moss on 9/30/22.
//

#include "series_tests.h"

namespace dropout_dl {
    tests test_series_name_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = series::get_series_name;

        std::string base_test_solution = "Base Test Title";
        std::string base_test = "<h1 class=\"collection-title\">\n" +
                                    base_test_solution +
                                "\n</h1>";

        out.emplace_back("Basic Series Name Parsing", test_function, base_test, base_test_solution);


        std::string multiple_header_test_solution = "Multi Header Test Title";
        std::string multiple_header_test = "<h1>\n"
                                           "Header without class or strong\n"
                                           "</h1>\n"
                                           "<h1 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small video-title\">\n"
                                           "Header with incorrect classes"
                                           "</h1>\n"
                                           "<h1 class=\"collection-title\">\n"
                                            + multiple_header_test_solution +
                                            "</h1>\n"
                                            "<h1 class=\"collection-title\">\n"
                                            "Valid Header After Correct Title\n"
                                            "</h1>";


        out.emplace_back("Multi Header Series Name Parsing", test_function, multiple_header_test, multiple_header_test_solution);


        std::string no_valid_name_test_solution = "ERROR";
        std::string no_valid_name_test = "<h1>\n"
                                           "Header without class or strong\n"
                                           "</h1>\n"
                                           "<h1 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small video-title\">\n"
                                           "Header with incorrect classes"
                                           "</h1>\n";


        out.emplace_back("No Valid Series Name Parsing", test_function, no_valid_name_test, no_valid_name_test_solution);



        std::string html_character_test_solution = "'&;";
        std::string html_character_test = "<h1 class=\"collection-title\">\n"
                                          "     &#39;&#38;&#59;\n"
                                          "</h1>";


        out.emplace_back("Html Character Series Name Parsing", test_function, html_character_test, html_character_test_solution);


        return {"Series Name Parsing", out};
    }
}

std::vector<dropout_dl::tests> test_series() {
    return {dropout_dl::test_series_name_parsing()};
}