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
			/// The number of the season
			int season_number;
			/// The link to the season page
			std::string url;
			/// The season page data
			std::string page_data;
			/// The list of all the episodes in the season
			std::vector<episode> episodes;

			episode get_episode(const std::string& html_data, int& start_point, const std::vector<cookie>& cookies, int episode_number = 0);

			/**
			 *
			 * @param html_data - The season page data
			 * @param cookies - The browser cookies
			 * @return A vector of all episodes in the season
			 *
			 * Gets all the episodes of the season and returns in a vector
			 */
			std::vector<episode> get_episodes(const std::vector<cookie>& cookies);

			/**
			 *
			 * @param url - The url of the season
			 * @return The number of the season
			 *
			 * Gets the canonical number of the season for the url. This is sometimes different from the displayed number because of special seasons.
			 */
			 static int get_season_number(const std::string& url);

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
				this->season_number = get_season_number(this->url);
				this->name = name;
				this->series_name = series_name;
				std::cout << series_name << ": " << name << ": " << "\n";
				this->page_data = get_generic_page(url);
				this->episodes = get_episodes(cookies);
			}
	};

} // dropout_dl

