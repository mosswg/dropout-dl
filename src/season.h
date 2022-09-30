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
            /// The name of the season
            std::string name;
            /// The name of the series
            std::string series_name;
            /// The link to the season page
            std::string url;
            /// The season page data
            std::string page_data;
            /// The list of all the episodes in the season
            std::vector<episode> episodes;

            /**
             *
             * @param html_data - The season page data
             * @param cookies - The browser cookies
             * @return A vector of all episodes in the season
             *
             * Gets all the episodes of the season and returns in a vector
             */
            static std::vector<episode> get_episodes(const std::string& html_data, const std::vector<cookie>& cookies);

            /**
             *
             * @param quality - The quality of the videos
             * @param series_directory - The directory of the series
             *
             * Downloads all the episodes of the season. Appends the season to the series directory
             */
            void download(const std::string& quality, const std::string& series_directory);

            /**
             *
             * @param url - The url to the webpage of the season
             * @param name - The name of the season
             * @param cookies - The browser cookies
             * @param series_name - The name of the series
             *
             * Creates a season object and populates the needed information.
             */
            season(const std::string& url, const std::string& name, const std::vector<cookie>& cookies, const std::string& series_name = "") {
                this->url = url;
                this->name = name;
                this->series_name = series_name;
                std::cout << series_name << ": " << name << ": " << url << "\n";
                this->page_data = get_generic_page(url);
                this->episodes = get_episodes(page_data, cookies);
            }
    };

} // dropout_dl

