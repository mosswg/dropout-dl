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
std::vector<std::string> get_cookies_from_firefox(bool verbose = false) {

    std::fstream firefox_profile_file("firefox_profile");
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
std::vector<std::string> get_cookies_from_chrome(bool verbose = false) {

    std::fstream chrome_profile_file("chrome_profile");
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

std::vector<std::string> get_cookies_from_files(bool verbose = false) {
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

#ifdef DROPOUT_DL_GCRYPT
std::vector<std::string> get_cookies(bool verbose = false) {

    if (std::filesystem::is_regular_file("firefox_profile_")) {
        return get_cookies_from_firefox(verbose);
    } else if (std::filesystem::is_regular_file("chrome_profile")) {
        return get_cookies_from_chrome(verbose);
    }
    else if (std::filesystem::is_regular_file("auth_cookie") && std::filesystem::is_regular_file("session_cookie")){
        return get_cookies_from_files(verbose);
    }
    else {
        std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
        exit(1);
    }
}
#elif defined(DROPOUT_DL_SQLITE)
std::vector<std::string> get_cookies(bool verbose = false) {

    if (std::filesystem::is_character_file("firefox_profile")) {
        return get_cookies_from_firefox(verbose);
    }
    else if (std::filesystem::is_character_file("auth_cookie") && std::filesystem::is_character_file("session_cookie")){
        return get_cookies_from_files(verbose);
    }
    else {
        std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
    }
}
#else
std::vector<std::string> get_cookies(bool verbose = false) {
    if (std::filesystem::is_character_file("auth_cookie") && std::filesystem::is_character_file("session_cookie")){
        return get_cookies_from_files(verbose);
    }
    else {
        std::cerr << "ERROR: dropout.tv cookies could not be found" << std::endl;
    }
}
#endif


int main(int argc, char** argv) {

    bool verbose = true;
    std::string quality = "1080p";

    std::string firefox_profile;
    std::string chrome_profile;

    std::string episode_url;

    std::vector<std::string> cookies;

    std::string video_data;

    if (argc > 1) {
        episode_url = argv[1];
        if (verbose) {
            std::cout << "Got episode url: " << episode_url << " from program arguments\n";
        }
    }
    else {
        std::cout << "Enter episode url: ";
        std::cin >> episode_url;
    }

    cookies = get_cookies(verbose);

    dropout_dl::episode ep(episode_url, cookies, verbose);

    std::string video_url = ep.get_video_url(quality);
    if (verbose) {
        std::cout << "Got video url: " << video_url << '\n';
    }


    if (!std::filesystem::is_directory(ep.series)) {
        std::filesystem::create_directories(ep.series);
        if (verbose) {
            std::cout << "Creating series directory" << '\n';
        }
    }

    std::fstream out(ep.filename, std::ios_base::in|std::ios_base::out|std::ios_base::trunc);

    out << ep.get_video_data(quality) << std::endl;

    return 0;
}
