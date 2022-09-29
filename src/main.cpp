#include <iostream>
#include <fstream>
#include <filesystem>

#include "episode.h"

#ifdef DROPOUT_DL_SQLITE
#include <sqlite3.h>
#ifdef DROPOUT_DL_GCRYPT
#include <gcrypt.h>
#endif
#endif

static int sqlite_write_callback(void* data, int argc, char** argv, char** azColName)
{
    if (argc < 1) {
        std::cerr << "ERROR: sqlite could not find dropout.tv cookie" << std::endl;
        return -1;
    }
    else {
        *(std::string*)data = argv[0];
        return 0;
    }
}

#ifdef DROPOUT_DL_SQLITE
std::vector<std::string> get_cookies_from_firefox(const std::filesystem::path& firefox_profile_path, bool verbose = false) {

    std::fstream firefox_profile_file(firefox_profile_path);
    std::string firefox_profile;

    std::string auth_cookie;
    std::string session_cookie;

    std::vector<std::string> out;

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

        char *err_code = nullptr;

        std::string sql("SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='__cf_bm';");

        rc = sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &auth_cookie, &err_code);

        out.emplace_back(auth_cookie);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_code);
            sqlite3_free(err_code);
            sqlite3_close(db);
            exit(2);
        } else if (verbose) {
            std::cout << "Got __cf_bm cookie from firefox sqlite db\n";
        }

        sql = "SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='_session';";

        rc = sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &session_cookie, &err_code);

        out.emplace_back(session_cookie);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_code);
            sqlite3_free(err_code);
            sqlite3_close(db);
            exit(3);
        } else if (verbose) {
            std::cout << "Got _session cookie from firefox sqlite db\n";
        }
        sqlite3_close(db);
    }

    return out;
}

#ifdef DROPOUT_DL_GCRYPT
std::vector<std::string> get_cookies_from_chrome(const std::filesystem::path& chrome_profile_path, bool verbose = false) {

    std::fstream chrome_profile_file(chrome_profile_path);
    std::string chrome_profile;

    std::string auth_cookie;
    int auth_cookie_length;
    std::string session_cookie;
    int session_cookie_length;

    std::vector<std::string> out;

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

        char *err_code = nullptr;

        std::string len;

        std::string sql = "SELECT length(encrypted_value) FROM cookies WHERE host_key LIKE '%dropout.tv%' AND name='__cf_bm';";

        sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &len, &err_code);

        auth_cookie_length = std::stoi(len);

        sql = "SELECT encrypted_value FROM cookies WHERE host_key LIKE '%dropout.tv%' AND name='__cf_bm';";

        rc = sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &auth_cookie, &err_code);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_code);
            sqlite3_free(err_code);
            sqlite3_close(db);
            exit(2);
        } else if (verbose) {
            std::cout << "Got __cf_bm cookie from chrome sqlite db\n" << auth_cookie << '\n';
        }

        sql = "SELECT length(encrypted_value) FROM cookies WHERE host_key LIKE '%dropout.tv%' AND name='_session';";

        sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &len, &err_code);

        session_cookie_length = std::stoi(len);

        sql = "SELECT encrypted_value FROM cookies WHERE host_key LIKE '%dropout.tv%' AND name='_session';";

        rc = sqlite3_exec(db, sql.c_str(), sqlite_write_callback, &session_cookie, &err_code);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_code);
            sqlite3_free(err_code);
            sqlite3_close(db);
            exit(3);
        } else if (verbose) {
            std::cout << "Got _session cookie from chrome sqlite db\n";
        }
        sqlite3_close(db);

        // system(("echo \"SELECT value FROM moz_cookies WHERE originAttributes LIKE '%dropout.tv%';\" | sqlite3 " + firefox_profile + "/cookies.sqlite > cookie").c_str());
    }

    // For mac os this is your keychain password
    // For linux leave as "peanuts"
    std::string password = "peanuts";
    std::string salt = "saltysalt";
    int length = 16;
    int iterations = 1;

    uint8_t key[32];

    char output[2048];

    char iv[16];

    for (char& c : iv) {
        c = ' ';
    }

    for (char& c : output) {
        c = 0;
    }

    for (int i = 0; i < auth_cookie_length; i++) {
        std::cout << std::hex << (0xFF & (int)auth_cookie[i]) << ' ';
    }
    std::cout << '\n';

    gcry_kdf_derive(password.c_str(), password.size(), GCRY_KDF_PBKDF2, GCRY_KDF_ARGON2ID, salt.c_str(), salt.size(), iterations, length, key);

    gcry_cipher_hd_t handle;

    gcry_cipher_open(&handle, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);

    gcry_cipher_setkey(handle, (const void*) &key, length);

    gcry_cipher_setiv(handle, (const void*)&iv, 16);

    unsigned long err = gcry_cipher_decrypt(handle, (unsigned char*)output, 2048, auth_cookie.c_str() + 3, auth_cookie_length - 3);

    if (err) {
        std::cout << gcry_strerror(err) << std::endl;
        exit(2);
    }

    for (char& c : output) {
        if (c == '\017') {
            c = 0;
        }
    }

    out.emplace_back(output);

    gcry_cipher_setiv(handle, (const void*)&iv, 16);

    gcry_cipher_decrypt(handle, (unsigned char*)output, 2048, session_cookie.c_str() + 3, session_cookie_length - 3);

    out.emplace_back(output);

    return out;
}
#endif
#endif

std::vector<std::string> get_cookies_from_files(const std::filesystem::path& auth_cookie_path, const std::filesystem::path& session_cookie_path, bool verbose = false) {
    std::fstream auth_cookie_file("auth_cookie");
    std::fstream session_cookie_file("session_cookie");

    std::string auth_cookie;
    std::string session_cookie;

    std::vector<std::string> out;

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

std::vector<std::string> get_cookies(bool verbose = false) {

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

/*
 * <select class="js-switch-season btn-dropdown-transparent margin-right-small" data-switch-season="">
                  <option value="https://www.dropout.tv/game-changer/season:1" selected="">
                    Season 1
                  </option>
                  <option value="https://www.dropout.tv/game-changer/season:2">
                    Season 2
                  </option>
                  <option value="https://www.dropout.tv/game-changer/season:3">
                    Season 3
                  </option>
                  <option value="https://www.dropout.tv/game-changer/season:4">
                    Season 4
                  </option>
                  <option value="https://www.dropout.tv/game-changer/season:7">
                    Bonus Content
                  </option>
              </select>
 */

class options {
public:

    std::string url;
    bool verbose = false;
    bool cookies_forced = false;
    std::string quality;
    std::vector<std::string> cookies;

    static std::vector<std::string> convert_program_args(int argc, char** argv) {
        std::vector<std::string> out;
        for (int i = 1; i < argc; i++) {
            out.emplace_back(argv[i]);
        }
        return out;
    }

    options(int argc, char** argv) {
        std::vector<std::string> args = convert_program_args(argc, argv);

        bool next_is_quality = false;
        bool next_is_cookie = false;
        for (const auto& arg : args) {
            if (next_is_quality) {
                quality = arg;
                next_is_quality = false;
                continue;
            }
            if (next_is_cookie) {
                cookies.emplace_back(arg);
                if (cookies.size() == 2) {
                    next_is_quality = false;
                }
                continue;
            }
            else if (arg.substr(0, 2) != "--") {
                url = arg;
            }
            else {
                if (arg == "--verbose") {
                    verbose = true;
                } else if (arg == "--quality") {
                    next_is_quality = true;
                }
                else if (arg == "--force-cookies") {
                    next_is_cookie = true;
                    cookies_forced = true;
                }
                else if (arg == "--help") {
                    std::cout << "Usage: dropout-dl [OPTIONS] <url> [OPTIONS]\n"
                                 "\n"
                                 "Options:\n"
                                    "\t--help\t\t\t\tDisplay this message\n"
                                    "\t--quality\t\t\tSet the quality of the downloaded video. Quality can be set to 'all' which\n"
                                    "\t\t\t\t\t\twill download all qualities and place them into separate folders\n"
                                    "\t--verbose\t\t\tDisplay debug information while running\n"
                                    "\t--force-cookies\t\tInterpret the next to arguments as authentication cookie and session cookie\n"
                                    << std::endl;

                    exit(0);
                }
            }
        }

        if (quality.empty()) {
            quality = "1080p";
        }
    }
};


int main(int argc, char** argv) {

    options options(argc, argv);

    std::cout << "quality: " << options.quality << std::endl;
    std::cout << "verbose: " << options.verbose << std::endl;
    std::cout << "url: \"" << options.url << '"' << std::endl;

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

    dropout_dl::episode ep(options.url, options.cookies, options.verbose);

    if (!std::filesystem::is_directory(ep.series)) {
        std::filesystem::create_directories(ep.series);
        if (options.verbose) {
            std::cout << "Creating series directory" << '\n';
        }
    }

    if (options.quality == "all") {
        for (const auto& possible_quality : ep.qualities) {
            if (!std::filesystem::is_directory(ep.series + "/" + possible_quality)) {
                std::filesystem::create_directories(ep.series + "/" + possible_quality);
                if (options.verbose) {
                    std::cout << "Creating series directory" << '\n';
                }
            }
            std::fstream out(ep.series + "/" + possible_quality + "/" + ep.filename,
                             std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

            out << ep.get_video_data(possible_quality) << std::endl;
        }
    }
    else {
        std::fstream out(ep.series + "/" + ep.filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

        out << ep.get_video_data(options.quality) << std::endl;
    }

    return 0;
}
