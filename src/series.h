//
// Created by moss on 9/29/22.
//
#pragma once

#include <iostream>
#include <vector>

#include "season.h"

namespace dropout_dl {


    /// A class for handling all series information and functions.
    class series {
        public:
            /// The name of the series
            std::string name;
            /// The link to the series page
            std::string url;
            /// The series page data
            std::string page_data;
            /// The directory which will contain the seasons of the series
            std::string series_directory;
            /// A vector containing all the season that this series include
            std::vector<season> seasons;

            /**
             *
             * @param html_data - The series page data
             * @return The name of the series
             *
             * Scrapes the series page for the name of the series
             */
            static std::string get_series_name(const std::string& html_data);

            /**
             *
             * @param html_data - The series page data
             * @param cookies - The cookies from a browser
             * @return A list of all seasons in the series
             *
             * Scrapes the series page for the names and link of all the season. Creates season objects for each of these.
             * These season object contain all the episodes of the season as episode objects.
             * The cookies this function takes are passed to the episode objects.
             */
            static std::vector<season> get_seasons(const std::string& html_data, const std::vector<cookie>& cookies);

            /**
             *
             * @param url - The url to the season
             * @param cookies - The browser cookies
             * @return A season object
             *
             * Gets the season page, which is really just a series page, and creates a season object with all the episodes of the season
             */
            static season get_season(const std::string& url, const std::vector<cookie>& cookies);

            /**
             *
             * @param quality - The quality of the video
             * @param base - The base directory to download to
             *
             * Downloads the series into the <b>base</b> directory with the format <i>\<base\>/\<series name\>/\<season name\>/\<episode\></i>
             */
           void download(const std::string& quality, const std::string& base = ".");

           /**
            *
            * @param url - The link to the series page
            * @param cookies - The browser cookies
            *
            * Creates a series object and populates the needed variables
            */
            explicit series(const std::string& url, const std::vector<dropout_dl::cookie>& cookies) {
                this->url = url;
                this->page_data = get_generic_page(url);
                this->name = get_series_name(page_data);
                if (name == "ERROR") {
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
