#include <iostream>

#include "series.h"

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
        bool cookies_forced = false;
        bool series = false;
        bool season = false;
        std::string quality;
        std::string filename;
        std::string output_directory;
        std::string episode;
        std::vector<cookie> cookies;

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

                if (arg.substr(0, 2) != "--") {
                    url = arg;
                    continue;
                }
                arg = arg.substr(2);
                if (arg == "verbose") {
                    verbose = true;
                } else if (arg == "quality") {
                    if (i + 1 >= args.size()) {
                        std::cerr << "ARGUMENT PARSE ERROR: --quality used with too few following arguments\n";
                        exit(8);
                    }
                    quality = args[++i];
                }
                else if (arg == "force-cookies") {
                    if (i + 2 >= args.size()) {
                        std::cerr << "ARGUMENT PARSE ERROR: --force-cookies used with too few following arguments\n";
                        exit(8);
                    }
                    cookies.emplace_back(args[++i]);
                    cookies.emplace_back(args[++i]);
                    cookies_forced = true;
                }
                else if (arg == "output") {
                    if (i + 1 >= args.size()) {
                        std::cerr << "ARGUMENT PARSE ERROR: --output used with too few following arguments\n";
                        exit(8);
                    }
                    filename = args[++i];
                }
                else if (arg == "output-directory") {
                    if (i + 1 >= args.size()) {
                        std::cerr << "ARGUMENT PARSE ERROR: --output-directory used with too few following arguments\n";
                        exit(8);
                    }
                    output_directory = args[++i];
                }
                else if (arg == "series") {
                    series = true;
                }
                else if (arg == "season") {
                    season = true;
                }
                else if (arg == "help") {
                    std::cout << "Usage: dropout-dl [OPTIONS] <url> [OPTIONS]\n"
                                 "\n"
                                 "Options:\n"
                                 "\t--help                   Display this message\n"
                                 "\t--quality                Set the quality of the downloaded video. Quality can be set to 'all' which\n"
                                 "\t                             will download all qualities and place them into separate folders\n"
                                 "\t--output                 Set the output filename. Only works for single episode downloads\n"
                                 "\t--output-directory       Set the directory where files are output\n"
                                 "\t--verbose                Display debug information while running\n"
                                 "\t--force-cookies          Interpret the next to arguments as authentication cookie and session cookie\n"
                                 "\t--series                 Interpret the url as a link to a series and download all episodes from all seasons\n"
                                 "\t--season                 Interpret the url as a link to a season and download all episodes from all seasons\n"
                              << std::endl;

                    exit(0);
                }
            }

            if (output_directory.empty()) {
                output_directory = ".";
            }

            if (season && series) {
                std::cerr << "ARGUMENT PARSE ERROR: Season and Series arguments used\n";
            }
            if (quality.empty()) {
                quality = "1080p";
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
std::vector<dropout_dl::cookie> get_cookies_from_firefox(const std::filesystem::path& firefox_profile_path, bool verbose = false) {

    std::fstream firefox_profile_file(firefox_profile_path);
    std::string firefox_profile;

    dropout_dl::cookie auth("__cf_bm");
    dropout_dl::cookie session("_session");

    std::vector<dropout_dl::cookie> out;

    firefox_profile_file >> firefox_profile;

    if (std::filesystem::is_directory(firefox_profile)) {

        sqlite3 *db;

        if (verbose) {
            std::cout << "Getting firefox cookies from firefox sqlite db\n";
        }

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

        auth.get_value_from_db(db, "FROM moz_cookies WHERE host LIKE '%dropout.tv%'", "value");

        session.get_value_from_db(db, "FROM moz_cookies WHERE host LIKE '%dropout.tv%'", "value");

        sqlite3_close(db);

        std::filesystem::remove("tmp/firefox_cookies.sqlite");

        if (std::filesystem::is_empty("tmp")) {
            std::filesystem::remove("tmp/");
        }
    }
    else {
        std::cerr << "FIREFOX COOKIE ERROR: Attempted to get cookies from firefox without profile." << std::endl;
        exit(4);
    }

    if (verbose) {
        std::cout << auth.name << ": " << auth.len << ": " << auth.value << '\n';

        std::cout << session.name << ": " << session.len << ": " << session.value << '\n';
    }

    out.push_back(auth);
    out.push_back(session);


    return out;
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
std::vector<dropout_dl::cookie> get_cookies_from_chrome(const std::filesystem::path& chrome_profile_path, bool verbose = false) {

    std::fstream chrome_profile_file(chrome_profile_path);
    std::string chrome_profile;

    dropout_dl::cookie auth("__cf_bm");
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

        auth.get_value_from_db(db, "FROM cookies WHERE host_key LIKE '%dropout.tv%'", "encrypted_value");

        session.get_value_from_db(db, "FROM cookies WHERE host_key LIKE '%dropout.tv%'", "encrypted_value");

        sqlite3_close(db);

    }
    else {
        std::cerr << "CHROME COOKIE ERROR: Attempted to get cookies from chrome without profile." << std::endl;
        exit(4);
    }

    auth.chrome_decrypt();

    session.chrome_decrypt();

    if (verbose) {
        std::cout << auth.name << ": " << auth.len << ": " << auth.value << '\n';

        std::cout << session.name << ": " << session.len << ": " << session.value << '\n';
    }

    out.push_back(auth);
    out.push_back(session);

    return out;
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
std::vector<dropout_dl::cookie> get_cookies(bool verbose = false) {

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

    {
        std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
        exit(7);
    }
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

    if (!options.cookies_forced) {
        options.cookies = get_cookies(options.verbose);
    }

    if (options.series) {
        dropout_dl::series series(options.url, options.cookies);

        series.download(options.quality, options.output_directory);
    }
    else if (options.season) {
        dropout_dl::season season = dropout_dl::series::get_season(options.url, options.cookies);

        std::string series_directory = dropout_dl::format_filename(season.series_name);

        std::cout << "ser: " << season.series_name << "\ndir: " << series_directory << '\n';

        season.download(options.quality, options.output_directory + "/" + series_directory);
    }
    else {
        dropout_dl::episode ep(options.url, options.cookies, options.verbose);

        if (options.verbose) {
            std::cout << "filename: " << options.filename << '\n';
        }

        if (!std::filesystem::is_directory(options.output_directory)) {
            std::filesystem::create_directories(options.output_directory);
            if (options.verbose) {
                std::cout << "Creating series directory" << '\n';
            }
        }

        if (options.filename.empty()) {
            options.filename = dropout_dl::format_filename(ep.name + ".mp4");
        }

        ep.download(options.quality, options.output_directory, options.filename);
    }


    return 0;
}
