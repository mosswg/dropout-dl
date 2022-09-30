#include <iostream>

#include "series.h"

#ifdef DROPOUT_DL_SQLITE
#include <sqlite3.h>
#endif

namespace dropout_dl {

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

        static std::vector<std::string> convert_program_args(int argc, char** argv) {
            std::vector<std::string> out;
            for (int i = 1; i < argc; i++) {
                out.emplace_back(argv[i]);
            }
            return out;
        }

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
    }

    if (verbose) {
        std::cout << auth.name << ": " << auth.len << ": " << auth.str << '\n';

        std::cout << session.name << ": " << session.len << ": " << session.str << '\n';
    }

    out.push_back(auth);
    out.push_back(session);


    return out;
}

#ifdef DROPOUT_DL_GCRYPT
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

    auth.chrome_decrypt();

    session.chrome_decrypt();

    session.url_decode();

    if (verbose) {
        std::cout << auth.name << ": " << auth.len << ": " << auth.str << '\n';

        std::cout << session.name << ": " << session.len << ": " << session.str << '\n';
    }

    out.push_back(auth);
    out.push_back(session);

    return out;
}
#endif
#endif

std::vector<dropout_dl::cookie> get_cookies_from_files(const std::filesystem::path& auth_cookie_path, const std::filesystem::path& session_cookie_path, bool verbose = false) {
    std::fstream auth_cookie_file("auth_cookie");
    std::fstream session_cookie_file("session_cookie");

    std::string auth_cookie;
    std::string session_cookie;

    std::vector<dropout_dl::cookie> out;

    auth_cookie_file >> auth_cookie;
    if (verbose) {
        std::cout << "Got __cf_bm cookie from auth_cookie file db\n";
    }

    out.emplace_back(auth_cookie);

    session_cookie_file >> session_cookie;
    if (verbose) {
        std::cout << "Got _session cookie from auth_cookie file db\n";
    }

    out.emplace_back(session_cookie);

    return out;
}

std::vector<dropout_dl::cookie> get_cookies(bool verbose = false) {

#ifdef DROPOUT_DL_SQLITE
    std::filesystem::path firefox_profile("firefox_profile");
#ifdef DROPOUT_DL_GCRYPT
    std::filesystem::path chrome_profile("chrome_profile");
#endif
#endif
    std::filesystem::path auth_cookie("auth_cookie");
    std::filesystem::path session_cookie("session_cookie");


    #ifdef DROPOUT_DL_SQLITE
    if (std::filesystem::exists(firefox_profile)) {
        return get_cookies_from_firefox(firefox_profile, verbose);
    } else
    #ifdef DROPOUT_DL_GCRYPT
    if (std::filesystem::exists(chrome_profile)) {
        return get_cookies_from_chrome(chrome_profile, verbose);
    } else
    #endif
    #endif
    if (std::filesystem::exists(auth_cookie) && std::filesystem::exists(session_cookie)){
        return get_cookies_from_files(auth_cookie, session_cookie, verbose);
    }
    else {
        std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
        exit(7);
    }
}


int main(int argc, char** argv) {
    dropout_dl::options options(argc, argv);

//    std::cout << "quality: " << options.quality << std::endl;
//    std::cout << "verbose: " << options.verbose << std::endl;
//    std::cout << "url: \"" << options.url << '"' << std::endl;

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

        std::string series_directory = season.series_name;

        std::replace(series_directory.begin(), series_directory.end(), ' ', '_');

        std::replace(series_directory.begin(), series_directory.end(), ',', '_');

        season.download(options.quality, options.output_directory + "/" + series_directory);
    }
    else {
        dropout_dl::episode ep(options.url, options.cookies, options.verbose);

        if (options.filename.empty()) {
            options.filename = "S" + (ep.season_number.size() < 2 ? "0" + ep.season_number : ep.season_number) + "E" +
                               (ep.episode_number.size() < 2 ? "0" + ep.episode_number : ep.episode_number) + ep.name +
                               ".mp4";

            std::replace(options.filename.begin(), options.filename.end(), ' ', '_');

            std::replace(options.filename.begin(), options.filename.end(), ',', '_');
        }

        if (options.verbose) {
            std::cout << "filename: " << options.filename << '\n';
        }

        if (!std::filesystem::is_directory(ep.series)) {
            std::filesystem::create_directories(ep.series);
            if (options.verbose) {
                std::cout << "Creating series directory" << '\n';
            }
        }

        ep.download(options.quality, options.output_directory + "/" + ep.series, options.filename);
    }


    return 0;
}
