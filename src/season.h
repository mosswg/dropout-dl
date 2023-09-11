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
			/// Whether or not to download captions
			bool download_captions;
            /// Whether to skip the video and only download captions
            bool download_captions_only;

			episode get_episode(const std::string& html_data, int& start_point, const cookie& session_cookie);


			/**
			 *
			 * @param session_cookie - The cookie used to authenticate
			 * @param page_data - the data of the season page gotten from curl
			 * @param episodes - the vector of episodes that will be appended to
			 */
			void add_episodes_to_vector(const cookie& session_cookie, const std::string& page_data, std::vector<episode>& episodes);

			/**
			 *
			 * @param html_data - The season page data
			 * @param session_cookie - The cookie used to authenticate
			 * @return A vector of all episodes in the season
			 *
			 * Gets all the episodes of the season and returns in a vector
			 */
			std::vector<episode> get_episodes(const cookie& session_cookie);

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
			 * @param session_cookie - The cookie used to authenticate
			 * @param series_name - The name of the series
			 *
			 * Creates a season object and populates the needed information.
			 */
			season(const std::string& url, const std::string& name, const cookie& session_cookie, const std::string& series_name = "", bool download_captions = false, bool download_captions_only = false) {
				this->url = url;
				this->download_captions = download_captions;
                this->download_captions_only = download_captions_only;
				this->season_number = get_season_number(this->url);
				this->name = name;
				this->series_name = series_name;
				std::cout << series_name << ": " << name << ": " << "\n";
				this->page_data = get_generic_page(url);
				this->episodes = get_episodes(session_cookie);
			}
	};

} // dropout_dl

