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
#include <sqlite3.h>
#ifdef DROPOUT_DL_GCRYPT
#include <gcrypt.h>
#endif

namespace dropout_dl {

    class cookie {
    public:
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

        static int sqlite_write_callback_uchar(void* data, int argc, char** argv, char** azColName)
        {
            if (argc < 1) {
                std::cerr << "ERROR: sqlite could not find dropout.tv cookie" << std::endl;
                return -1;
            }
            else {

                auto* ck = (dropout_dl::cookie*)data;

                for (int i = 0; i < ck->len; i++) {
                    if (argv[0][i] > 32 && argv[0][i] < 126) {
                        std::cout << (unsigned char) argv[0][i] << ' ';
                    }
                    else {
                        std::cout << std::hex << ((int)argv[0][i] & 0xFF) << ' ';
                    }
                    ck->str[i] = (unsigned char)argv[0][i];
                }
                return 0;
            }
        }


        std::string name;
        std::string str;
        int len;


        explicit cookie(const std::string& name) {
            this->name = name;
            this->len = 0;
        }

        cookie(const std::string& name, const std::string& cookie) {
            this->name = name;
            this->str = cookie;
            this->len = cookie.size();
        }

        cookie(const std::string& cookie, int length) {
            this->str = cookie;
            this->name = "?";
            this->len = length;
        }

        cookie(const std::string& name, const std::string& cookie, int length) {
            this->name = name;
            this->str = cookie;
            this->len = length;
        }

        /**
         *
         * @param db - An sqlite3 database
         * @param sql_query_base - A base without the name search e.g. "FROM cookies" this function would then append the text "SELECT <value>" and "WHERE name='<name>'"
         * @param value - The name of the value to fill the cookie with
         *
         *
         */
        void get_value_from_db(sqlite3* db, const std::string& sql_query_base, const std::string& value, bool verbose = false, int (*callback)(void*,int,char**,char**) = sqlite_write_callback);

        void format_from_chrome();

        /**
         *
         * @param password - Default is "peanuts". This works for linux. The password should be keychain password on MacOS
         * @param salt - Salt is "saltysalt" for both MacOS and Linux
         * @param length - Length of 16 is standard for both MacOS and Linux
         * @param iterations - 1 on linux and 1003 on MacOS
         */
        void chrome_decrypt(const std::string& password = "peanuts", int iterations = 1, const std::string& salt = "saltysalt", int length = 16);

        void url_decode();
    };


    bool substr_is(const std::string& string, int start, const std::string& test_str);

    void replace_all(std::string& str, const std::string& from, const std::string& to);

    #if defined(__WIN32__)
    #include <windows.h>
    msec_t time_ms(void);
    #else
    #include <sys/time.h>
    long time_ms();
    #endif

    static int curl_progress_func(void* ptr, double total_to_download, double downloaded, double total_to_upload, double uploaded);

    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    std::string get_generic_page(const std::string& url, bool verbose = false);

    class episode {

    public:
        std::string series;
        std::string name;
        std::string episode_number;
        std::string season_number;
        std::string episode_url;
        std::string episode_data;
        std::string embedded_url;
        std::string embedded_page_data;
        std::string config_url;
        std::string config_data;
        std::vector<std::string> qualities;
        std::vector<std::string> quality_urls;

        bool verbose = false;

        // Curl

        static std::string get_episode_page(const std::string& url, const std::string& auth_cookie, const std::string& session_cookie, bool verbose = false);

        static std::string get_embedded_page(const std::string& url, const std::string& cookie, bool verbose = false);


        // Parsing
        static std::string get_series_name(const std::string& html_data);

        static std::string get_episode_name(const std::string& html_data);

        static std::string get_episode_number(const std::string& html_data);

        static std::string get_season_number(const std::string& html_data);

        static std::string get_embed_url(const std::string& html_data);

        static std::string get_config_url(const std::string& html_data);

        std::vector<std::string> get_qualities();

        std::string get_video_url(const std::string& quality);

        std::string get_video_data(const std::string& quality);

        void download(const std::string& quality, const std::string& series_directory, std::string filename = "");

        episode(const std::string& episode_url, std::vector<cookie> cookies, bool verbose = false) {

            this->episode_url = episode_url;
            this->verbose = verbose;

            episode_data = get_episode_page(episode_url, cookies[0].str, cookies[1].str);

            name = get_episode_name(episode_data);

            if (verbose) {
                std::cout << "Got name: " << name << '\n';
            }

            if (name == "ERROR") {
                std::cerr << "EPISODE ERROR: Invalid Episode URL\n";
                exit(6);
            }

            this->season_number = get_season_number(episode_data);

            if (verbose) {
                std::cout << "Got season: " << this->season_number << '\n';
            }

            this->episode_number = get_episode_number(episode_data);

            if (verbose) {
                std::cout << "Got episode: " << this->episode_number << '\n';
            }

            this->series = get_series_name(episode_data);

            if (verbose) {
                std::cout << "Got series: " << this->series << '\n';
            }

            std::replace(this->series.begin(), this->series.end(), ' ', '_');

            std::replace(this->series.begin(), this->series.end(), ',', '_');

            this->embedded_url = get_embed_url(episode_data);

            replace_all(this->embedded_url, "&amp;", "&");

            if (verbose) {
                std::cout << "Got embedded url: " << this->embedded_url << '\n';
            }

            this->embedded_page_data = get_embedded_page(this->embedded_url, cookies[0].str);

            if (this->embedded_page_data.find("you are not authorized") != std::string::npos) {
                std::cerr << "ERROR: Could not access video. Try refreshing cookies.\n";
                exit(6);
            }

            this->config_url = get_config_url(this->embedded_page_data);

            replace_all(this->config_url, "\\u0026", "&");

            if (verbose) {
                std::cout << "Got config url: " << this->embedded_url << '\n';
            }

            this->config_data = get_generic_page(this->config_url);

            this->get_qualities();
        }

        episode() = default;
    };

} // dropout_dl
