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
            std::string series_directory;
            std::vector<season> seasons;

            static std::string get_series_name(const std::string& html_data);

            static std::vector<season> get_seasons(const std::string& html_data, const std::vector<cookie>& cookies);

            static season get_season(const std::string& url, const std::vector<cookie>& cookies);

           void download(const std::string& quality, const std::string& base);

            explicit series(const std::string& url, const std::vector<dropout_dl::cookie>& cookies) {
                this->url = url;
                this->page_data = get_generic_page(url);
                this->name = get_series_name(page_data);
                if (name == "-1") {
                    std::cerr << "SERIES PARSE ERROR: Could not parse series name\n";
                    exit(10);
                }

                this->series_directory = name;
                std::replace(this->series_directory.begin(), this->series_directory.end(), ' ', '_');
                std::replace(this->series_directory.begin(), this->series_directory.end(), ',', '_');

                this->seasons = get_seasons(page_data, cookies);
            }
    };

} // dropout_dl
