//
// Created by moss on 9/29/22.
//
#pragma once

#include <iostream>
#include <vector>

#include "season.h"

namespace dropout_dl {

    class series {
        public:
            std::string name;
            std::string url;
            std::string page_data;
            std::vector<season> seasons;

            static std::string get_series_name(const std::string& html_data);

            static std::vector<season> get_seasons(const std::string& html_data, const std::vector<std::string>& cookies);

            static season get_season(const std::string& url, const std::vector<std::string>& cookies);

           void download(const std::string& quality, const std::string& series_directory);

            explicit series(const std::string& url, const std::vector<std::string>& cookies) {
                this->url = url;
                this->page_data = get_generic_page(url);
                this->name = get_series_name(page_data);
                if (name == "-1") {
                    std::cerr << "SERIES PARSE ERROR: Could not parse series name\n";
                    exit(10);
                }
                this->seasons = get_seasons(page_data, cookies);
            }
    };

} // dropout_dl
