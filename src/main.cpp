#include <iostream>

#include "series.h"
#include "login.h"
#include <regex>

#ifdef DROPOUT_DL_SQLITE
#include <sqlite3.h>
#endif

namespace dropout_dl {

	/**
	 * A class for handling and storing the program arguments.
	 */
	class options {
	public:

		std::string url;
		bool verbose = false;
		bool force_cookies = false;
		bool browser_cookies = false;
		bool is_series = false;
		bool is_season = false;
		bool is_episode = false;
		bool download_captions = false;
        bool download_captions_only = false;
		std::string quality;
		std::string filename;
		std::string output_directory;
		std::string episode;
		cookie session_cookie;

		/**
		 *
		 * @param argc - The number of provided program arguments
		 * @param argv - The provided program arguments
		 * @return A vector of arguments in the c++ string format
		 *
		 * Converts the C style program arguments to a vector of strings
		 */
		static std::vector<std::string> convert_program_args(int argc, char** argv) {
			std::vector<std::string> out;
			for (int i = 1; i < argc; i++) {
				out.emplace_back(argv[i]);
			}
			return out;
		}

		/**
		 *
		 * @param argc - The number of provided program arguments
		 * @param argv - The provided program arguments
		 *
		 * Parses and handles the program arguments and creates an options object.
		 */
		options(int argc, char** argv) {
			std::vector<std::string> args = convert_program_args(argc, argv);

			for (int i = 0; i < args.size(); i++) {
				std::string arg = args[i];

				if (arg[0] != '-') {
					url = arg;
					continue;
				}
				if (arg[1] == '-') {
					// Full names. prefixed by `--`
					arg = arg.substr(2);
				}
				else {
					// Shorthands. prefixed by `-`
					arg = arg.substr(1);
				}
				if (arg == "verbose" || arg == "v") {
					verbose = true;
				} else if (arg == "quality" || arg == "q") {
					if (i + 1 >= args.size()) {
						std::cerr << "ARGUMENT PARSE ERROR: --quality used with too few following arguments\n";
						exit(8);
					}
					quality = args[++i];
				}
				else if (arg == "browser-cookies" || arg == "bc") {
					browser_cookies = true;
				}
				else if (arg == "force-cookies") {
					if (i + 2 >= args.size()) {
						std::cerr << "ARGUMENT PARSE ERROR: --force-cookies used with too few following arguments\n";
						exit(8);
					}
					session_cookie = cookie(args[++i]);
					force_cookies = true;
				}
				else if (arg == "output" || arg == "o") {
					if (i + 1 >= args.size()) {
						std::cerr << "ARGUMENT PARSE ERROR: --output used with too few following arguments\n";
						exit(8);
					}
					filename = args[++i];
				}
				else if (arg == "output-directory" || arg == "d") {
					if (i + 1 >= args.size()) {
						std::cerr << "ARGUMENT PARSE ERROR: --output-directory used with too few following arguments\n";
						exit(8);
					}
					output_directory = args[++i];
				}
				else if (arg == "series" || arg == "S") {
					is_series = true;
				}
				else if (arg == "season" || arg == "s") {
					is_season = true;
				}
				else if (arg == "episode" || arg == "e") {
					is_episode = true;
				}
				else if (arg == "captions" || arg == "c") {
					download_captions = true;
				}
                else if (arg == "captions-only" || arg == "co") {
					download_captions_only = true;
				}
				else if (arg == "help" || arg == "h") {
					std::cout << "Usage: dropout-dl [OPTIONS] <url> [OPTIONS]\n"
								 "\n"
								 "Options:\n"
								 "\t--help              -h   Display this message\n"
								 "\t--quality           -q   Set the quality of the downloaded video. Quality can be set to 'all' which\n"
								 "\t                             will download all qualities and place them into separate folders\n"
								 "\t--output            -o   Set the output filename. Only works for single episode downloads\n"
								 "\t--output-directory  -d   Set the directory where files are output\n"
								 "\t--verbose           -v   Display debug information while running\n"
								 "\t--browser-cookies   -bc  Use cookies from the browser placed in 'firefox_profile' or 'chrome_profile'\n"
								 "\t--force-cookies          Interpret the next to argument as the session cookie\n"
								 "\t--series            -S   Interpret the url as a link to a series and download all episodes from all seasons\n"
								 "\t--season            -s   Interpret the url as a link to a season and download all episodes from all seasons\n"
								 "\t--episode           -e   Interpret the url as a link to a single episode\n"
								 "\t--captions          -c   Download the captions along with the episode\n"
                                 "\t--captions-only     -co  Download the captions only, without the episode\n";

					exit(0);
				}
			}

			if (output_directory.empty()) {
				output_directory = ".";
			}

			if (browser_cookies && force_cookies) {
				std::cerr << "ARGUMENT PARSE ERROR: Cannot use browser cookies and forced cookies\n";
				// Default to browser cookies.
				force_cookies = false;
			}

			if ((is_season && is_series) || (is_season && is_episode) || (is_series && is_episode)) {
				std::cerr << "ARGUMENT PARSE ERROR: Mulitple parse type arguments used\n";
			}
			if (quality.empty()) {
				quality = "1080p";
			}

			if (!(is_season || is_series || is_episode)) {
				std::regex season_regex("season:\\d+\\/?$", std::regex::ECMAScript);
				std::regex episode_regex("/videos/", std::regex::ECMAScript);
				if (std::regex_search(url, season_regex)) {
					is_season = true;
				}
				else if (std::regex_search(url, episode_regex)) {
					is_episode = true;
				}
				else {
					is_series = true;
				}
			}
		}
	};
}

#ifdef DROPOUT_DL_SQLITE
/**
 *
 * @param firefox_profile_path - The path to a firefox profile
 * @param verbose - Whether or not to be verbose
 * @return A vector of cookies
 *
 * Gets the needed cookies from the firefox sqlite database associated with the path provided.
 */
dropout_dl::cookie get_cookies_from_firefox(const std::filesystem::path& firefox_profile_path, bool verbose = false) {

	std::fstream firefox_profile_file(firefox_profile_path);
	std::string firefox_profile;

	dropout_dl::cookie session("_session");

	std::vector<dropout_dl::cookie> out;

	firefox_profile_file >> firefox_profile;

	if (!std::filesystem::is_directory(firefox_profile)) {
		std::cerr << "FIREFOX COOKIE ERROR: Attempted to get cookies from firefox without profile." << std::endl;
		exit(4);
	}


	sqlite3 *db;

	if (verbose) {
		std::cout << "Getting firefox cookies from firefox sqlite db\n";
	}

	/// Firefox locks the database so we have to make a copy.
	if (!std::filesystem::is_directory("tmp"))
		std::filesystem::create_directories("tmp");
	std::filesystem::remove("tmp/firefox_cookies.sqlite");
	std::filesystem::copy_file(firefox_profile + "/cookies.sqlite", "tmp/firefox_cookies.sqlite");

	int rc = sqlite3_open("tmp/firefox_cookies.sqlite", &db);
	if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
		exit(1);
	} else {
		if (verbose) {
			std::cout << "Firefox database opened successfully\n";
		}
	}

	std::string len;

	session.get_value_from_db(db, "FROM moz_cookies WHERE host LIKE '%dropout.tv%'", "value");

	sqlite3_close(db);

	std::filesystem::remove("tmp/firefox_cookies.sqlite");

	if (std::filesystem::is_empty("tmp")) {
		std::filesystem::remove("tmp/");
	}

	if (verbose) {
		std::cout << session.name << ": " << session.len << ": " << session.value << '\n';
	}

	return session;
}

#ifdef DROPOUT_DL_GCRYPT
/**
 *
 * @param chrome_profile_path - The path to a chrome profile
 * @param verbose - Whether or not to be verbose
 * @return A vector of cookies
 *
 * Gets the needed cookies from the chrome sqlite database associated with the path provided and decrypts them using the libgcrypt library.
 * This function does not work for windows and must be modified slightly for mac os.
 * For mac os the calls to cookie::chrome_decrypt must be passed the parameters detailed in it's documentation.
 */
dropout_dl::cookie get_cookies_from_chrome(const std::filesystem::path& chrome_profile_path, bool verbose = false) {

	std::fstream chrome_profile_file(chrome_profile_path);
	std::string chrome_profile;

	dropout_dl::cookie session("_session");

	std::vector<dropout_dl::cookie> out;

	getline(chrome_profile_file, chrome_profile);

	if (std::filesystem::is_directory(chrome_profile)) {

		sqlite3 *db;

		if (verbose) {
			std::cout << "Getting chrome cookies from chrome sqlite db\n";
		}

		int rc = sqlite3_open((chrome_profile + "/Cookies").c_str(), &db);
		if (rc) {
			std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
			exit(1);
		} else {
			if (verbose) {
				std::cout << "Chrome database opened successfully\n";
			}
		}

		std::string len;

		session.get_value_from_db(db, "FROM cookies WHERE host_key LIKE '%dropout.tv%'", "encrypted_value");

		sqlite3_close(db);

	}
	else {
		std::cerr << "CHROME COOKIE ERROR: Attempted to get cookies from chrome without profile." << std::endl;
		exit(4);
	}

	session.chrome_decrypt();

	if (verbose) {
		std::cout << session.name << ": " << session.len << ": " << session.value << '\n';
	}

	return session;
}
#endif
#endif

/**
 *
 * @param verbose - Whether or not to be verbose
 * @return A vector of cookies
 *
 * Determines whether to get cookies from firefox or chrome. This function should not be run if cookies are forced using the `--force-cookies` option.
 * This function checks firefox first so if both firefox and chrome profiles are provided it will use firefox.
 */
dropout_dl::cookie get_cookie_from_browser(bool verbose = false) {

	std::filesystem::path firefox_profile("firefox_profile");
	std::filesystem::path chrome_profile("chrome_profile");

	if (std::filesystem::exists(firefox_profile)) {

		#ifdef DROPOUT_DL_SQLITE
		return get_cookies_from_firefox(firefox_profile, verbose);
		#else
				std::cout << "WARNING: Firefox profile file exists but sqlite is not installed" << std::endl;
		#endif
	}
	if (std::filesystem::exists(chrome_profile)) {
		#if defined(DROPOUT_DL_GCRYPT) & defined(DROPOUT_DL_SQLITE)
		return get_cookies_from_chrome(chrome_profile, verbose);
		#else
		std::cout << "WARNING: Chrome profile file exists but libgcrypt or sqlite is not installed" << std::endl;
		#endif
	}

	std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
	exit(7);
}


int main(int argc, char** argv) {
	dropout_dl::options options(argc, argv);

	if (options.verbose) {
		std::cout << "quality: " << options.quality << std::endl;
		std::cout << "verbose: " << options.verbose << std::endl;
		std::cout << "url: \"" << options.url << '"' << std::endl;
	}

	std::string firefox_profile;
	std::string chrome_profile;

	std::string video_data;

	if (options.url.empty()) {
		std::cout << "Enter episode url: ";
		std::cin >> options.url;
	}
	else if (options.verbose) {
		std::cout << "Got episode url: " << options.url << " from program arguments\n";
	}

	if (options.browser_cookies) {
		options.session_cookie = get_cookie_from_browser(options.verbose);
	}
	else if (!options.force_cookies) {
		std::string session;
		dropout_dl::login::get_cookies(session);

		options.session_cookie = dropout_dl::cookie("_session", session);
	}

	if (options.is_series) {
		if (options.verbose) {
			std::cout << "Getting series\n";
		}
		dropout_dl::series series(options.url, options.session_cookie, options.download_captions, options.download_captions_only);

		series.download(options.quality, options.output_directory);
	}
	else if (options.is_season) {
		if (options.verbose) {
			std::cout << "Getting season\n";
		}
		dropout_dl::season season = dropout_dl::series::get_season(options.url, options.session_cookie, options.download_captions, options.download_captions_only);

		season.download(options.quality, options.output_directory + "/" + season.series_name);
	}
	else if (options.is_episode) {
		if (options.verbose) {
			std::cout << "Getting episode\n";
		}
		dropout_dl::episode ep(options.url, options.session_cookie, options.verbose, options.download_captions, options.download_captions_only);

		if (options.verbose) {
			std::cout << "filename: " << options.filename << '\n';
		}

		if (!std::filesystem::is_directory(options.output_directory)) {
			std::filesystem::create_directories(options.output_directory);
			if (options.verbose) {
				std::cout << "Creating series directory" << '\n';
			}
		}

		ep.download(options.quality, options.output_directory, options.filename);
	}
	else {
		std::cerr << "ERROR: Could not determine parsing type\n";
	}


	return 0;
}
