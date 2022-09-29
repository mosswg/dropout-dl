//
// Created by moss on 9/28/22.
//

#include "episode.h"

namespace dropout_dl {

    // dropout-dl helpers
    bool substr_is(const std::string& string, int start, const std::string& test_str) {
        if (test_str.size() != test_str.size())
            return false;

        for (int i = start, j = 0; i < start + test_str.size(); i++, j++) {
            if (string[i] != test_str[j]) {
                return false;
            }
        }
        return true;
    }

    void replace_all(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
    }

    #if defined(__WIN32__)
        #include <windows.h>
        msec_t time_ms(void)
        {
            return timeGetTime();
        }
    #else
    #include <sys/time.h>
        long time_ms()
        {
            timeval tv{};
            gettimeofday(&tv, nullptr);
            return tv.tv_sec * 1000 + tv.tv_usec / 1000;
        }
    #endif


    long current_time;
    long last_progress_timestamp;

    int curl_progress_func(void* ptr, double total_to_download, double downloaded, double total_to_upload, double uploaded) {
        const double number_chars = 50;
        const char* full_character = "▓";
        const char* empty_character = "░";

        current_time = time_ms();
        if (current_time - 50 > last_progress_timestamp) {
            double percent_done = (downloaded / total_to_download) * number_chars;
            double percent_done_clone = percent_done;
            putchar('[');
            while (percent_done_clone-- > 0) {
                std::cout << full_character;
            }
            while (percent_done++ < number_chars) {
                std::cout << empty_character;
            }
            putchar(']');
            putchar(' ');
            std::cout << downloaded / 1048576 << "MiB / " << total_to_download / 1048576 << "MiB                           ";
            putchar('\r');
            last_progress_timestamp = time_ms();
            std::cout.flush();
        }
        return 0;
    }

    // episode statics
    std::string episode::get_series_name(const std::string& html_data) {
        std::string series_title("series-title");
        std::string open_a_tag("<a");
        std::string close_tag(">");
        std::string close_a("</a>");

        int series_name_start = -1;

        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, series_title)) {
                for (int j = i + series_title.size(); j < html_data.size(); j++) {
                    if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                    if (substr_is(html_data, j, open_a_tag)) {
                        for (int k = j + open_a_tag.size(); k < html_data.size(); k++) {
                            if (substr_is(html_data, k, close_tag)) {
                                for (int l = 0; l < html_data.size() - k; l++) {
                                    char c = html_data[k + l];
                                    if (series_name_start == -1) {
                                        if (html_data[k + l + 1] == '\n' || html_data[k + l + 1] == ' ' ||
                                            html_data[k + l + 1] == '\t') {
                                            continue;
                                        } else {
                                            series_name_start = k + l + 1;
                                        }
                                    }
                                    if (substr_is(html_data, k + l, close_a) || (series_name_start != -1 && html_data[k + l] == '\n')) {
                                        return html_data.substr(series_name_start, l - (series_name_start - k));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return "-1";
    }

    std::string episode::get_episode_name(const std::string& html_data) {
        int title_start = -1;
        std::string video_title("video-title");
        std::string open_strong("<strong>");
        std::string close_strong("</strong>");
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, video_title)) {
                for (int j = i; j < html_data.size(); j++) {
                    if (substr_is(html_data, j, open_strong)) {
                        title_start = j + 8;
                        break;
                    }
                }
                for (int j = 0; j < html_data.size() - title_start; j++) {
                    if (substr_is(html_data, title_start + j, close_strong)) {
                        return html_data.substr(title_start, j);
                    }
                }
            }
        }
        return "ERROR";
    }

    std::string episode::get_episode_number(const std::string& html_data) {
        std::string episode("Episode");
        std::string close_a("</a>");
        std::string episode_num;
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, episode)) {
                for (int j = i + 8; j < html_data.size(); j++) {
                    if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                    if (substr_is(html_data, j, close_a)) {
                        return episode_num;
                    }
                    episode_num += html_data[j];
                }
            }
        }
        return "-1";
    }

    std::string episode::get_season_number(const std::string& html_data) {
        std::string season("Season");
        std::string dash(",");
        std::string season_num;
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, season)) {
                for (int j = i + 7; j < html_data.size(); j++) {
                    if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                    if (html_data[j] == '-' || html_data[j] == ',' ) {
                        return season_num;
                    }
                    season_num += html_data[j];
                }
            }
        }
        return "-1";
    }

    std::string episode::get_embed_url(const std::string& html_data) {
        std::string config("window.VHX.config");
        std::string embed_url("embed_url: ");
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, config)) {
                for (int j = i + config.size(); j < html_data.size(); j++) {
                    if (substr_is(html_data, j, embed_url)) {
                        for (int k = 0; k < html_data.size(); k++) {
                            if (html_data[k + j + embed_url.size() + 1] == '"') {
                                return html_data.substr(j + embed_url.size() + 1, k);
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

    std::string episode::get_config_url(const std::string& html_data) {
        std::string OTTdata("OTTData");
        std::string config_url("\"config_url\"");
        int remaining_quotes = 1;
        int url_start = -1;
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, OTTdata)) {
                for (int j = i + OTTdata.size(); j < html_data.size(); j++) {
                    if (substr_is(html_data, j, config_url)) {
                        for (int k = 0; k < html_data.size() - (i + OTTdata.size()); k++) {
                            char c = html_data[j + k + config_url.size()];
                            if (remaining_quotes != 0) {
                                if (html_data[j + k + config_url.size()] == '"') {
                                    remaining_quotes--;
                                }
                                continue;
                            }
                            else if (url_start == -1) {
                                url_start = j + k + config_url.size();
                            }

                            if (html_data[url_start + k] == '"') {
                                return html_data.substr(url_start, k);
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

    std::string episode::get_episode_page(const std::string& url, const std::string& auth_cookie, const std::string& session_cookie, bool verbose) {
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1;

        std::string episode_data;

        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:101.0) Gecko/20100101 Firefox/101.0");
        slist1 = curl_slist_append(slist1, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
        slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en;q=0.5");
        slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
        slist1 = curl_slist_append(slist1, "DNT: 1");
        slist1 = curl_slist_append(slist1, "Connection: keep-alive");
        slist1 = curl_slist_append(slist1, ("Cookie: locale_det=en; referrer_url=https%3A%2F%2Fwww.dropout.tv%2Fgame-changer; _session=" + session_cookie + "; __stripe_mid=3dd96b43-2e51-411d-8614-9f052c92d8ba0506a7; _device=X11%3AFirefox%3A1u9pxwBcfaKsXubmTnNbfA; __cf_bm=" + auth_cookie + "; tracker=%7B%22country%22%3A%22us%22%2C%22platform%22%3A%22linux%22%2C%22uid%22%3A1048462031243%2C%22site_id%22%3A%2236348%22%7D").c_str());
        slist1 = curl_slist_append(slist1, "Upgrade-Insecure-Requests: 1");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: document");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: navigate");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
        slist1 = curl_slist_append(slist1, "Sec-GPC: 1");

        hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.84.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_VERBOSE, verbose);

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &episode_data);
        /* Here is a list of options the curl code used that cannot get generated
           as source easily. You may choose to either not use them or implement
           them yourself.

        CURLOPT_WRITEDATA set to a objectpointer
        CURLOPT_INTERLEAVEDATA set to a objectpointer
        CURLOPT_WRITEFUNCTION set to a functionpointer
        CURLOPT_READDATA set to a objectpointer
        CURLOPT_READFUNCTION set to a functionpointer
        CURLOPT_SEEKDATA set to a objectpointer
        CURLOPT_SEEKFUNCTION set to a functionpointer
        CURLOPT_ERRORBUFFER set to a objectpointer
        CURLOPT_STDERR set to a objectpointer
        CURLOPT_HEADERFUNCTION set to a functionpointer
        CURLOPT_HEADERDATA set to a objectpointer

        */

        ret = curl_easy_perform(hnd);

        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return episode_data;
    }

    std::string episode::get_embedded_page(const std::string& url, const std::string& cookie, bool verbose) {
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1;
        std::string embedded_page;

        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:101.0) Gecko/20100101 Firefox/101.0");
        slist1 = curl_slist_append(slist1, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
        slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en;q=0.5");
        slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
        slist1 = curl_slist_append(slist1, "DNT: 1");
        slist1 = curl_slist_append(slist1, "Connection: keep-alive");
        slist1 = curl_slist_append(slist1, "Referer: https://www.dropout.tv/");
        slist1 = curl_slist_append(slist1, "Cookie: __cf_bm=Ayc3uSgUEf9kJ20sfVBLgdo5fvloLmSLWBkJtzzhZR8-1662831290-0-ASVO2Fg9txI6nslt2tle7Y2MjRw4sI8/gFRbMDI8vHIP0nhb1SDk1I7lF5hWK9RMGP9wOFJwyqThLXQkuTj9m2c=");
        slist1 = curl_slist_append(slist1, "Upgrade-Insecure-Requests: 1");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: iframe");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: navigate");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
        slist1 = curl_slist_append(slist1, "Sec-GPC: 1");

        hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.84.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_VERBOSE, verbose);


        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &embedded_page);
        /* Here is a list of options the curl code used that cannot get generated
           as source easily. You may choose to either not use them or implement
           them yourself.

        CURLOPT_WRITEDATA set to a objectpointer
        CURLOPT_INTERLEAVEDATA set to a objectpointer
        CURLOPT_WRITEFUNCTION set to a functionpointer
        CURLOPT_READDATA set to a objectpointer
        CURLOPT_READFUNCTION set to a functionpointer
        CURLOPT_SEEKDATA set to a objectpointer
        CURLOPT_SEEKFUNCTION set to a functionpointer
        CURLOPT_ERRORBUFFER set to a objectpointer
        CURLOPT_STDERR set to a objectpointer
        CURLOPT_HEADERFUNCTION set to a functionpointer
        CURLOPT_HEADERDATA set to a objectpointer

        */

        ret = curl_easy_perform(hnd);

        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return embedded_page;
    }

    std::string episode::get_config_page(const std::string& url, bool verbose) {
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1;

        std::string config_page;

        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:101.0) Gecko/20100101 Firefox/101.0");
        slist1 = curl_slist_append(slist1, "Accept: */*");
        slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en;q=0.5");
        slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
        slist1 = curl_slist_append(slist1, "Referer: https://embed.vhx.tv/");
        slist1 = curl_slist_append(slist1, "Origin: https://embed.vhx.tv");
        slist1 = curl_slist_append(slist1, "DNT: 1");
        slist1 = curl_slist_append(slist1, "Connection: keep-alive");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: empty");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: cors");
        slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
        slist1 = curl_slist_append(slist1, "Sec-GPC: 1");

        hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.84.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_VERBOSE, verbose);

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &config_page);

        /* Here is a list of options the curl code used that cannot get generated
           as source easily. You may choose to either not use them or implement
           them yourself.

        CURLOPT_WRITEDATA set to a objectpointer
        CURLOPT_INTERLEAVEDATA set to a objectpointer
        CURLOPT_WRITEFUNCTION set to a functionpointer
        CURLOPT_READDATA set to a objectpointer
        CURLOPT_READFUNCTION set to a functionpointer
        CURLOPT_SEEKDATA set to a objectpointer
        CURLOPT_SEEKFUNCTION set to a functionpointer
        CURLOPT_ERRORBUFFER set to a objectpointer
        CURLOPT_STDERR set to a objectpointer
        CURLOPT_HEADERFUNCTION set to a functionpointer
        CURLOPT_HEADERDATA set to a objectpointer

        */

        ret = curl_easy_perform(hnd);

        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return config_page;
    }

    std::vector<std::string> episode::get_qualities() {
        if (!qualities.empty()) {
            return qualities;
        }
        int i = 0;
        bool video_section = false;

        const std::string quality_marker = R"("quality":")";
        for (; i < config_data.size(); i++ ) {
            // std::cout << i << "/" << javascript_data.size() << ": " << javascript_data[i] << ": " << javascript_data.substr(i, 17) << ": " << video_section << "\n";
            if (config_data.substr(i, 9) == "video/mp4") {
                video_section = true;
            }

            if (video_section && config_data.substr(i, quality_marker.size()) == quality_marker) {
                i += quality_marker.size();
                for (int j = 0; j + i < config_data.size(); j++) {
                    if (config_data[i + j] == '"') {
                        this->qualities.push_back(config_data.substr(i, j));
                        if (this->verbose) {
                            std::cout << "Found quality (" << i << " + " << j << "): " << qualities.back() << std::endl;
                        }
                        break;
                    }
                }
                for (int j = i; j > 0; j--) {
                    // std::cout << i << ": " << javascript_data[i] << ": " << javascript_data.substr(i-7, 7) << "\n";
                    if (this->config_data.substr(j-7, 7) == R"("url":")") {
                        for (int k = 0; k < i - j; k++) {
                            if (config_data[j + k] == '"') {
                                this->quality_urls.emplace_back(config_data.substr(j, k));
                                if (this->verbose) {
                                    std::cout << "Found url (" << j << " + " << k << "): " << quality_urls.back()
                                              << std::endl;
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
        return qualities;
    }


    std::string episode::get_video_url(const std::string& quality) {
        for (int i = 0; i < qualities.size(); i++) {
            if (qualities[i] == quality) {
                return quality_urls[i];
            }
        }
        std::cerr << "ERROR: quality of " << quality << " not found\nPossible qualities: ";
        for (int i = 0; i < qualities.size(); i++) {
            std::cerr << qualities[i];
            if (i != qualities.size() - 1) {
                std::cerr << ", ";
            }
        }
        exit(6);
    }

    std::string episode::get_video_data(const std::string &quality) {
        CURL* curl = curl_easy_init();
        CURLcode res;
        if(curl) {
            std::string out;

            curl_easy_setopt(curl, CURLOPT_URL, get_video_url(quality).c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropout_dl::episode::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, dropout_dl::curl_progress_func);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            return out;
        }
        return "CURL ERROR";
    }
} // dropout_dl