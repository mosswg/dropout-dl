//
// Created by moss on 9/29/22.
//

#include "series.h"

namespace dropout_dl {

    std::string series::get_series_name(const std::string& html_data) {
        std::string collection_title("collection-title");
        std::string open_a_tag("<h1");
        std::string close_tag(">");
        std::string close_a("</h1>");

        int series_name_start = -1;

        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, collection_title)) {
                for (int j = i + collection_title.size(); j < html_data.size(); j++) {
                    if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                    if (substr_is(html_data, j, close_tag)) {
                        for (int l = 0; l < html_data.size() - j; l++) {
                            char c = html_data[j + l];
                            if (series_name_start == -1) {
                                if (html_data[j + l + 1] == '\n' || html_data[j + l + 1] == ' ' ||
                                    html_data[j + l + 1] == '\t') {
                                    continue;
                                } else {
                                    series_name_start = j + l + 1;
                                }
                            }
                            if (substr_is(html_data, j + l, close_a) || (series_name_start != -1 && html_data[j + l] == '\n')) {
                                return html_data.substr(series_name_start, l - (series_name_start - j));
                            }
                        }
                    }
                }
            }
        }
        return "-1";
    }

    std::vector<season> series::get_seasons(const std::string &html_data, const std::vector<cookie>& cookies) {
        std::vector<season> out;

        std::string search_class("js-switch-season");
        std::string open_select("<select");
        std::string close_tag(">");
        std::string close_select("</select>");

        std::string open_option("<option");
        std::string close_option("</option>");
        std::string value("value=");

        bool seasons_dropdown = false;
        std::string season_url;
        std::string season_name;
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, open_select)) {
                for (int j = i; j < html_data.size(); j++) {
                    if (substr_is(html_data, j, search_class)) {
                        i = j;
                        seasons_dropdown = true;
                        break;
                    }
                    else if (substr_is(html_data, j, close_tag)) {
                        break;
                    }
                }
            }
            if (seasons_dropdown) {
                if (substr_is(html_data, i, value)) {
                    i += value.size() + 1;
                    for (int j = 0; j + i < html_data.size(); j++) {
                        if (html_data[i + j] == '"') {
                            season_url = html_data.substr(i, j);
                            i += j;
                            break;
                        }
                    }
                }
                else if (!season_url.empty() && substr_is(html_data, i, close_tag)) {
                    i += close_tag.size() + 1;
                    for (int j = 0; i + j < html_data.size(); j++) {
                        if (html_data[i + j] == '\n') {
                            season_name = html_data.substr(i, j);

                            // Remove leading and trailing whitespace
                            bool leading_whitespace = true;
                            int name_start;
                            int name_end;
                            for (int k = 0; k < season_name.size(); k++) {
                                if (season_name[k] != ' ' && season_name[k] != '\t' && season_name[k] != '\n') {
                                    name_start = k;
                                    break;
                                }
                            }
                            for (int k = season_name.size() - 1; k > 0; k--) {
                                if (season_name[k] != ' ' && season_name[k] != '\t' && season_name[k] != '\n') {
                                    name_end = k;
                                    break;
                                }
                            }
                            season_name = season_name.substr(name_start, season_name.size() - name_start - name_end);

                            out.emplace_back(season_url, season_name, cookies);

                            std::cout << out.back().name << ": " << out.back().url << '\n';

                            season_url.clear();
                            season_name.clear();

                            i = i + j;

                            break;
                        }
                    }
                }

                if (substr_is(html_data, i, close_select)) {
                    break;
                }
            }
        }

        return out;
    }


    season series::get_season(const std::string &url, const std::vector<cookie>& cookies) {
        std::string html_data = get_generic_page(url);

        std::string search_class("js-switch-season");
        std::string open_select("<select");
        std::string close_tag(">");
        std::string close_select("</select>");

        std::string open_option("<option");
        std::string close_option("</option>");
        std::string value("value=");

        bool seasons_dropdown = false;
        bool selected = false;
        std::string season_url;
        std::string season_name;
        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, open_select)) {
                for (int j = i; j < html_data.size(); j++) {
                    if (substr_is(html_data, j, search_class)) {
                        i = j;
                        seasons_dropdown = true;
                        break;
                    }
                    else if (substr_is(html_data, j, close_tag)) {
                        break;
                    }
                }
            }
            if (seasons_dropdown) {
                if (substr_is(html_data, i, value)) {
                    i += value.size() + 1;
                    for (int j = 0; j + i < html_data.size(); j++) {
                        if (html_data[i + j] == '"') {
                            season_url = html_data.substr(i, j);
                            i += j;
                            break;
                        }
                    }
                }
                else if (!season_url.empty() && substr_is(html_data, i, "selected")) {
                    selected = true;
                }
                else if (!season_url.empty() && substr_is(html_data, i, close_tag)) {
                    i += close_tag.size() + 1;
                    for (int j = 0; i + j < html_data.size(); j++) {
                        if (html_data[i + j] == '\n') {
                            if (selected) {
                                season_name = html_data.substr(i, j);

                                // Remove leading and trailing whitespace
                                bool leading_whitespace = true;
                                int name_start;
                                int name_end;
                                for (int k = 0; k < season_name.size(); k++) {
                                    if (season_name[k] != ' ' && season_name[k] != '\t' && season_name[k] != '\n') {
                                        name_start = k;
                                        break;
                                    }
                                }
                                for (int k = season_name.size() - 1; k > 0; k--) {
                                    if (season_name[k] != ' ' && season_name[k] != '\t' && season_name[k] != '\n') {
                                        name_end = k;
                                        break;
                                    }
                                }
                                season_name = season_name.substr(name_start,
                                                                 season_name.size() - name_start - name_end);

                                return {season_url, season_name, cookies, get_series_name(html_data)};
                            }

                            season_url.clear();
                            season_name.clear();

                            i = i + j;

                            break;
                        }
                    }
                }

                if (substr_is(html_data, i, close_select)) {
                    break;
                }
            }
        }

        std::cerr << "SEASON PARSE ERROR: No selected season found\n";
        exit(9);
    }

    void series::download(const std::string &quality, const std::string& base) {
        if (!std::filesystem::is_directory(base + "/" + series_directory)) {
            std::filesystem::create_directories(base + "/" + series_directory);
            std::cout << "Creating series directory" << '\n';
        }

        for (auto& season : seasons) {
            season.download(quality, base + "/" + series_directory);
        }
    }

} // dropout_dl