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

#include <nlohmann/json.hpp>

#include "color.h"
#include "cookie.h"
#include "util.h"

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
		nlohmann::json metadata;
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
		/// The json parsed data of the main config page.
		nlohmann::json config_json;
		/// The list of the video qualities available for the episode. This is a parallel array with the quality_urls vector
		std::vector<std::string> video_qualities;
		/// The list of the qualities available for the episode. This is a parallel array with the quality_urls vector
		std::vector<std::string> audio_qualities;
		/// The names of the files created. Should go: initial segment, video, audio, captions. The only strict part is the initial segment must come first
		std::vector<std::string> filenames;
		/// Base64 video initial segments
		std::vector<std::string> video_initial_segment_quality;
		/// Base64 audio initial segments
		std::vector<std::string> audio_initial_segment_quality;
		/// The list of the audio segment urls correlating with the qualities array.
		std::vector<std::vector<std::string>> audio_quality_segments;
		/// The list of the video segment urls correlating with the qualities array.
		std::vector<std::vector<std::string>> video_quality_segments;
		/// Whether to skip the video and only download captions
		bool download_captions_only;

		/// If the episode is being downloaded from CDN (this is needed because the audio and video files are seperated)
		bool is_from_cdn = false;

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
		static nlohmann::json get_meta_data_json(const std::string& html_data);

		// Parsing
		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the series
		 *
		 * Get the name of the series from the metadata
		 */
		static std::string get_series_name(const nlohmann::json& meta_data);


		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the season
		 *
		 * Get the name of the season from the metadata
		 */
		static std::string get_season_name(const nlohmann::json& meta_data);


		/**
		 *
		 * @param page_data - Episode page data
		 * @return The episode number
		 *
		 * Get the number of the season from the metadata
		 */
		static int get_episode_number(const std::string& page_data, int season_number);

		/**
		 *
		 * @param meta_data - Episode metadata in json format
		 * @return The name of the episode
		 *
		 * Get the name of the episode from the metadata
		 */
		static std::string get_episode_name(const nlohmann::json& meta_data);

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
		 * @return Whether the episode already exists on disk or not.
		 *
		 * Compare the <b>filename</b> aswell as the <b>filesize</b> of a to be downloaded episode with files on disk.
		 */
		bool check_existing(const std::string& quality, const std::string& filename = "");


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
		 * @return The index of that quality in the quality array
		 *
		 * <b>Quality</b> must be contained in the <b>qualities</b> vector otherwise this function will give an error and exit the program after listing the available qualities.
		 */
		int get_video_quality_index(const std::string& quality);

		/**
		 *
		 * @param quality - The quality of the video
		 * @param segment - Segment index
		 * @return The url to the video segment
		 *
		 * Get a link to the segment of the video at index <b>segment</b> of the episode with the given <b>quality</b>. <b>Quality</b> must be a valid index in the <b>qualities</b>.
		 */
		std::string get_video_segment_url(int quality, int segment);


		/**
		 *
		 * @param quality - The quality of the video
		 * @param segment - Segment index
		 * @return The url to the audio segment
		 *
		 * Get a link to the segment of the audio at index <b>segment</b> of the episode with the given <b>quality</b>. <b>Quality</b> must be a valid index into <b>qualities</b>.
		 */
		std::string get_audio_segment_url(int quality, int segment);

		/**
		 *
		 * @param quality - The quality of the video
		 * @param segment_index - The index of the segment
		 * @param curl_buffer - The buffer that the video segment will be written to via Curl
		 * @param filename - The filename which will be displayed will downloading the video
		 * @return True if it succeeded, false if it did not
		 *
		 * Download the segment of the episode with the given quality and segment_index and return the raw video data as a string. The <b>filename</b> parameter is only used for displaying while downloading the video so that the user knows what is being downloaded. The <b>filename</b> argument is entirely optional and this function will not place the video into a file whether the value is given or not.
		 */
		bool get_video_segment_data(int quality, int segment_index, std::string& curl_buffer, const std::string& filename = "");

		/**
		 *
		 * @param quality - The quality of the video
		 * @param segment_index - The index of the segment
		 * @param filename - The filename which will be displayed will downloading the video
		 * @return The video data
		 *
		 * Download the segment of the audio of the episode. <b>filename</b> is only used for progress display.
		 */
		std::string get_audio_segment_data(int quality, int segment_index, const std::string& filename = "");


		/**
		 *
		 * @param quality - The quality of the video
		 * @param base_directory - The directory which the episode is downloaded into
		 * @param filename - The name of the file (Will default if empty)
		 *
		 * Downloads the episode using the get_video_data function and places it into the <b>filename</b> file in the <b>base_directory</b> directory.
		 * If the file already exists it will output the name in yellow and will not redownload.
		 */
		void download_quality(const std::string& quality, const std::string& base_directory, const std::string& filename, bool lowest_audio_quality = false);

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
		episode(const std::string& episode_url, cookie session_cookie, const std::string& series, const std::string& season, int episode_number, int season_number, bool verbose = false, bool download_captions = false, bool download_captions_only = false) {
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

			int episode_number_from_page = get_episode_number(episode_data, season_number);
			if (episode_number_from_page != -1) {
				this->episode_number = episode_number_from_page;
			}
			else {
				this->episode_number = episode_number;
			}
			if (episode_number_from_page != episode_number) {
				if (verbose) {
					std::cout << "WARNING: episode number from season page (" << episode_number << ") and episode page (" << episode_number_from_page << ") do not match. Using " << this->episode_number << " if this is correct please ignore this warning\n";
				}
			}

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

			try {
				this->config_json = nlohmann::json::parse(config_data);
			}
			catch (nlohmann::detail::parse_error e) {
				if (this->config_data == "error code: 1015") {
					std::cout << "EPISODE ERROR: You are rate limited. Wait a few minutes, increase the -r flag, and try again\n";
					std::cout << "\tIf you are using the default rate please report this on github\n";
					exit(10);
				}
				else {
					std::cout << "EPISODE ERROR: could not parse json: " << config_data << "\n";
				}
			}

			if (download_captions || download_captions_only) {
				this->captions_url = get_captions_url();
				if (verbose) {
					std::cout << "Got caption url: " << this->captions_url << "\n";
				}
			}
			else {
				this->captions_url = "";
			}

			this->download_captions_only = download_captions_only;

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
		episode(const std::string& episode_url, const cookie& session_cookie, bool verbose = false, bool download_captions = false, bool download_captions_only = false) {

			this->episode_url = episode_url;
			this->verbose = verbose;
			episode_data = get_episode_page(episode_url, session_cookie.value);

			if (verbose) {
				std::cout << "Got page data\n";
			}

			metadata = get_meta_data_json(episode_data);

			if (metadata == "ERROR") {
				std::cerr << "EPISODE ERROR: Could not find metadata. Plesae ensure the episode URL is valid.\n";
				exit(6);
			}

			if (verbose) {
				std::cout << "Got episode metadata: " << metadata << '\n';
			}

			name = get_episode_name(metadata);

			if (verbose) {
				std::cout << "Got name: " << name << '\n';
			}

			if (name == "ERROR") {
				std::cerr << "EPISODE ERROR: Could not find episode name. Plesae ensure the episode URL is valid.\n";
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

			try {
				this->config_json = nlohmann::json::parse(config_data);
			}
			catch (nlohmann::detail::parse_error e) {
				if (this->config_data == "error code: 1015") {
					std::cout << "EPISODE ERROR: You are rate limited. Wait a few minutes, increase the -r flag, and try again\n";
					std::cout << "\tIf you are using the default rate please report this on github\n";
					exit(10);
				}
				else {
					std::cout << "EPISODE ERROR: could not parse json: " << config_data << "\n";
				}
			}

			if (download_captions || download_captions_only) {
				this->captions_url = get_captions_url();
			}
			else {
				this->captions_url = "";
			}

			this->download_captions_only = download_captions_only;

			this->get_qualities();
		}

		/**
		 * Creates an episode object with no data. This should only be used for invalid states.
		 */
		episode() = default;
	};

} // dropout_dl
