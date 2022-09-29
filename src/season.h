//
// Created by moss on 9/29/22.
//
#pragma once

#include <iostream>
#include <vector>

#include "episode.h"

namespace dropout_dl {

    class season {
        public:
            std::string name;
            std::string series_name;
            std::string url;
            std::string page_data;
            std::vector<episode> episodes;

            static std::vector<episode> get_episodes(const std::string& html_data, const std::vector<std::string>& cookies);

            void download(const std::string& quality, const std::string& series_directory);

            season(const std::string& url, const std::string& name, const std::vector<std::string>& cookies, const std::string& series_name = "") {
                this->url = url;
                this->name = name;
                this->series_name = series_name;
                std::cout << series_name << ": " << name << ": " << url << "\n";
                this->page_data = get_generic_page(url);
                this->episodes = get_episodes(page_data, cookies);
            }
    };

} // dropout_dl

