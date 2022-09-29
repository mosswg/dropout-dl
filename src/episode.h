//
// Created by moss on 9/28/22.
//
#pragma once

#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <algorithm>

namespace dropout_dl {

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
        std::string filename;
        std::vector<std::string> qualities;
        std::vector<std::string> quality_urls;

        bool verbose;

        // Curl

        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        static std::string get_episode_page(const std::string& url, const std::string& auth_cookie, const std::string& session_cookie, bool verbose = false);

        static std::string get_embedded_page(const std::string& url, const std::string& cookie, bool verbose = false);

        static std::string get_config_page(const std::string& url, bool verbose = false);


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


        explicit episode(const std::string& episode_url, std::vector<std::string> cookies, bool verbose = false) {
            this->episode_url = episode_url;
            this->verbose = verbose;

            episode_data = get_episode_page(episode_url, cookies[0], cookies[1]);

            name = get_episode_name(episode_data);

            if (verbose) {
                std::cout << "Got name: " << name << '\n';
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

            this->filename = this->series + "/S" + (this->season_number.size() < 2 ? "0" + this->season_number : this->season_number) + "E" + (this->episode_number.size() < 2 ? "0" + this->episode_number : this->episode_number) + this->name + ".mp4";

            std::replace(filename.begin(), filename.end(), ' ', '_');

            std::replace(filename.begin(), filename.end(), ',', '_');

            if (verbose) {
                std::cout << "filename: " << filename << '\n';
            }

            this->embedded_url = get_embed_url(episode_data);

            replace_all(this->embedded_url, "&amp;", "&");

            if (verbose) {
                std::cout << "Got embedded url: " << this->embedded_url << '\n';
            }

            this->embedded_page_data = get_embedded_page(this->embedded_url, cookies[0]);

            if (this->embedded_page_data.find("you are not authorized") != std::string::npos) {
                std::cerr << "ERROR: Could not access video. Try refreshing cookies.\n";
                exit(6);
            }

            this->config_url = get_config_url(this->embedded_page_data);

            replace_all(this->config_url, "\\u0026", "&");

            if (verbose) {
                std::cout << "Got config url: " << this->embedded_url << '\n';
            }

            this->config_data = get_config_page(this->config_url);

            this->get_qualities();
        }
    };

} // dropout_dl
