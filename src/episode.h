//
// Created by moss on 9/28/22.
//
#pragma once

#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>

#include "color.h"
#include "cookie.h"
#include "util.h"

#include <sqlite3.h>

namespace dropout_dl {
	/**
	 * A class for handling all episode information. This class is wildly overkill if downloading an entire series as it gather the series name and season for every episode. This is not an issue here because all the information it gathers it already available while gathering the video url and the majority of the time taken while parsing an episode is from downloading the three required webpages.
	 */
	class episode {

	public:
		/// The name of the series that the episode belongs to
		std::string series;
		/// The directory for the series
		std::string series_directory;
		/// The name of the season that the episode belongs to
		std::string season;
		/// The number of the season (only set when downloading a season or series)
		int season_number = 0;
		/// The json metadata of the episode
		std::string metadata;
		/// The name of the episode
		std::string name;
		/// The number of the episode (only set when downloading a season or series)
		int episode_number = -1;
		/// The url for the main episode page
		std::string episode_url;
		/// The data of the main episode page
		std::string episode_data;
		/// The url for the main embedded page. This contains page the link to the config page
		std::string embedded_url;
		/// The data of the main embedded page. This contains the link to the config page
		std::string embedded_page_data;
		/// The url for the main config page. This contains page the link to the mp4 video of the episode
		std::string config_url;
		/// The url for the captions of the episode.
		std::string captions_url;
		/// The data of the main config page. This contains the link to the mp4 video of the episode
		std::string config_data;
		/// The list of the qualities available for the episode. This is a parallel array with the quality_urls vector
		std::vector<std::string> qualities;
		/// The list of the urls correlating with the qualities array.
		std::vector<std::string> quality_urls;

		/// Whether or not to be verbose
		bool verbose = false;

		// Curl

		/**
		 *
		 * @param url - The url of the episode page
		 * @param session_cookie - The session cookie with name "_session".
		 * @param verbose - Whether or not to be verbose (not recommended)
		 * @return The episode page data
		 */
		static std::string get_episode_page(const std::string& url, const std::string& session_cookie, bool verbose = false);

		/**
		 *
		 * @param html_data - Episode page data
		 * @return The json data for the episode
		 *
		 * Gets the json metadata of the episode
		*/
		static std::string get_meta_data_json(const std::string& html_data);

		// Parsing
		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the series
		 *
		 * Get the name of the series from the metadata
		 */
		static std::string get_series_name(const std::string& meta_data);


		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the season
		 *
		 * Get the name of the season from the metadata
		 */
		static std::string get_season_name(const std::string& meta_data);

		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the episode
		 *
		 * Get the name of the episode from the metadata
		 */
		static std::string get_episode_name(const std::string& meta_data);

		/**
		 *
		 * @param html_data - Episode page data
		 * @return The url of the embedded page
		 *
		 * Get the url of the embedded page from the episode page
		 */
		static std::string get_embed_url(const std::string& html_data);

		/**
		 *
		 * @param html_data - Embedded page data
		 * @return The url of the config page
		 *
		 * Get the url of the config page from the embedded page data
		 */
		static std::string get_config_url(const std::string& html_data);

		/**
		 *
		 * @return A vector of qualities
		 *
		 * Gets the available qualities for the episode and populate the <b>qualities</b> and <b>quality_urls</b> vectors.
		 * If this function has already been run it simply returns the already populated <b>qualities</b> vector unless said vector has been cleared.
		 */
		std::vector<std::string> get_qualities();


		/**
		 *
		 * @return the url for the captions of the episode
		 *
		 * Gets the url for the captions of the episode if possible. If not returns "".
		 */
		std::string get_captions_url();

		/**
		 *
		 * @param quality - The quality of the video
		 * @return The url to the video
		 *
		 * Get a link to the video of the episode with the given <b>quality</b>. <b>Quality</b> must be contained in the <b>qualities</b> vector otherwise this function will give an error and exit the program after listing the available qualities.
		 */
		std::string get_video_url(const std::string& quality);

		/**
		 *
		 * @param quality - The quality of the video
		 * @param filename - The filename which will be displayed will downloading the video
		 * @return The video data
		 *
		 * Download the episode with the given quality and return the raw video data as a string. The <b>filename</b> parameter is only used for displaying while downloading the video so that the user knows what is being downloaded. The <b>filename</b> argument is entirely optional and this function will not place the video into a file whether the value is given or not.
		 */
		std::string get_video_data(const std::string& quality, const std::string& filename = "");


		/**
		 *
		 * @param quality - The quality of the video
		 * @param base_directory - The directory which the episode is downloaded into
		 * @param filename - The name of the file (Will default if empty)
		 *
		 * Downloads the episode using the get_video_data function and places it into the <b>filename</b> file in the <b>base_directory</b> directory.
		 * If the file already exists it will output the name in yellow and will not redownload.
		 */
		void download_quality(const std::string& quality, const std::string& base_directory, const std::string& filename);

		/**
		 *
		 * @param quality - The quality of the video
		 * @param series_directory - The directory which the episode is downloaded into
		 * @param filename - The name of the file (Will default if empty)
		 *
		 * Downloads the episode using the get_video_data function and places it into the <b>filename</b> file in the <b>series_directory</b> directory.
		 * If the <b>filename</b> parameter is left empty it will default to the E\<episode_number\>\<name\>.mp4 format.
		 */
		void download(const std::string& quality, const std::string& series_directory, std::string filename = "");



		/**
		 *
		 * @param episode_url - Link to the episode
		 * @param cookies - The current cookies from the browser
		 * @param series - The series the episode is in
		 * @param season - The season the episode is in
		 * @param episode_number - The number of the episode
		 * @param season_number - The number of the season the episode is in
		 * @param verbose - Whether or not be verbose
		 *
		 * Create an episode object from the link using to cookies to get all the necessary information.
		 * This constructor initializes all the object data.
		 */
		episode(const std::string& episode_url, cookie session_cookie, const std::string& series, const std::string& season, int episode_number, int season_number, bool verbose = false, bool download_captions = false) {
			this->episode_url = episode_url;
			this->verbose = verbose;

			episode_data = get_episode_page(episode_url, session_cookie.value);

			if (verbose) {
				std::cout << "Got page data\n";
			}

			metadata = get_meta_data_json(episode_data);

			if (verbose) {
				std::cout << "Got episode metadata: " << metadata << '\n';
			}

			name = get_episode_name(metadata);

			if (verbose) {
				std::cout << "Got name: " << name << '\n';
			}

			if (name == "ERROR") {
				std::cerr << "EPISODE ERROR: Invalid Episode URL\n";
				exit(6);
			}

			this->series = series;

			this->series_directory = format_filename(this->series);

			this->season = season;

			this->episode_number = episode_number;

			this->season_number = season_number;


			this->embedded_url = get_embed_url(episode_data);

			replace_all(this->embedded_url, "&amp;", "&");

			if (verbose) {
				std::cout << "Got embedded url: " << this->embedded_url << '\n';
			}

			this->embedded_page_data = get_generic_page(this->embedded_url);

			if (this->embedded_page_data.find("you are not authorized") != std::string::npos) {
				std::cerr << "ERROR: Could not access video. Try refreshing cookies.\n";
				exit(6);
			}

			this->config_url = get_config_url(this->embedded_page_data);

			replace_all(this->config_url, "\\u0026", "&");

			if (verbose) {
				std::cout << "Got config url: " << this->config_url << '\n';
			}

			this->config_data = get_generic_page(this->config_url);

			if (download_captions) {
				this->captions_url = get_captions_url();
				if (verbose) {
					std::cout << "Got caption url: " << this->captions_url << "\n";
				}
			}
			else {
				this->captions_url = "";
			}

			this->get_qualities();
			}

		/**
		 *
		 * @param episode_url - Link to the episode
		 * @param cookies - The current cookies from the browser
		 * @param verbose - Whether or not be verbose
		 *
		 * Create an episode object from the link using to cookies to get all the necessary information.
		 * This constructor initializes all the object data.
		 */
		episode(const std::string& episode_url, const cookie& session_cookie, bool verbose = false, bool download_captions = false) {

			this->episode_url = episode_url;
			this->verbose = verbose;
			episode_data = get_episode_page(episode_url, session_cookie.value);

			if (verbose) {
				std::cout << "Got page data\n";
			}

			metadata = get_meta_data_json(episode_data);

			if (verbose) {
				std::cout << "Got episode metadata: " << metadata << '\n';
			}

			name = get_episode_name(metadata);

			if (verbose) {
				std::cout << "Got name: " << name << '\n';
			}

			if (name == "ERROR") {
				std::cerr << "EPISODE ERROR: Invalid Episode URL\n";
				exit(6);
			}

			this->series = get_series_name(metadata);

			if (verbose) {
				std::cout << "Got series: " << this->series << '\n';
			}

			this->season = get_season_name(metadata);

			if (verbose) {
				std::cout << "Got season: " << this->season << '\n';
			}

			this->series_directory = format_filename(this->series);

			if (verbose) {
				std::cout << "Got series directory: " << this->series_directory << '\n';
			}

			this->embedded_url = get_embed_url(episode_data);

			replace_all(this->embedded_url, "&amp;", "&");

			if (verbose) {
				std::cout << "Got embedded url: " << this->embedded_url << '\n';
			}

			this->embedded_page_data = get_generic_page(this->embedded_url);

			if (this->embedded_page_data.find("you are not authorized") != std::string::npos) {
				std::cerr << "ERROR: Could not access video. Try refreshing cookies.\n";
				exit(6);
			}

			this->config_url = get_config_url(this->embedded_page_data);

			replace_all(this->config_url, "\\u0026", "&");

			if (verbose) {
				std::cout << "Got config url: " << this->config_url << '\n';
			}

			this->config_data = get_generic_page(this->config_url);

			if (download_captions) {
				this->captions_url = get_captions_url();
			}
			else {
				this->captions_url = "";
			}

			this->get_qualities();
		}

		/**
		 * Creates an episode object with no data. This should only be used for invalid states.
		 */
		episode() = default;
	};

} // dropout_dl
