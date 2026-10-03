//
// Created by slmingol
//

#include "season_tests.h"

namespace dropout_dl {

    // Minimal single-episode HTML fragment matching the site_video marker pattern
    static std::string make_episode_html(const std::string& href, const std::string& label) {
        return "<a href=\"" + href + "\" "
               "class=\"browse-item-link\" data-track-event=\"site_video\" "
               "data-track-event-properties=\"{&quot;label&quot;:&quot;" + label + "&quot;}\""
               ">\n</a>";
    }

    tests test_episode_list_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        // helper: parse and return "title\turl" for first result, or "ERROR"
        auto first_title = [](const std::string& html) -> std::string {
            auto list = season::parse_episode_list(html);
            return list.empty() ? "ERROR" : list[0].first;
        };
        auto first_url = [](const std::string& html) -> std::string {
            auto list = season::parse_episode_list(html);
            return list.empty() ? "ERROR" : list[0].second;
        };
        auto count = [](const std::string& html) -> std::string {
            return std::to_string(season::parse_episode_list(html).size());
        };

        // basic title extraction
        std::string ep1_href = "https://watch.dropout.tv/smartypants/season:3/videos/smartyshorts-nice-to-meet-you";
        std::string ep1_label = "Smartyshorts Nice to Meet You";
        std::string ep1_html = make_episode_html(ep1_href, ep1_label);

        out.emplace_back("Basic episode title parsing",
            first_title(ep1_html), ep1_label);

        out.emplace_back("Basic episode URL parsing",
            first_url(ep1_html), ep1_href);

        // missing label → skipped (count = 0)
        std::string no_label_html =
            "<a href=\"https://watch.dropout.tv/show/season:1/videos/no-title\" "
            "class=\"browse-item-link\" data-track-event=\"site_video\">\n</a>";

        out.emplace_back("Episode with missing label is skipped",
            count(no_label_html), std::string("0"));

        // multiple episodes
        std::string ep2_href = "https://watch.dropout.tv/game-changer/season:6/videos/new-mouth-who-dis";
        std::string ep2_label = "New Mouth Who Dis";
        std::string multi_html = make_episode_html(ep1_href, ep1_label) + "\n" +
                                 make_episode_html(ep2_href, ep2_label);

        out.emplace_back("Multiple episodes — count",
            count(multi_html), std::string("2"));

        out.emplace_back("Multiple episodes — second title",
            [&]() -> std::string {
                auto list = season::parse_episode_list(multi_html);
                return list.size() >= 2 ? list[1].first : "ERROR";
            }(), ep2_label);

        return {"Episode List Parsing", out};
    }

    tests test_decode_unicode_escapes() {
        std::vector<dropout_dl::test<std::string>> out;

        auto decode = [](const std::string& s) { return season::decode_unicode_escapes(s); };

        out.emplace_back("\\u0026 decoded to &",
            decode("Rock \\u0026 Roll"), std::string("Rock & Roll"));

        out.emplace_back("\\u0027 decoded to '",
            decode("Don\\u0027t Stop"), std::string("Don't Stop"));

        out.emplace_back("\\u003c decoded to <",
            decode("A \\u003c B"), std::string("A < B"));

        out.emplace_back("\\u003e decoded to >",
            decode("A \\u003e B"), std::string("A > B"));

        out.emplace_back("Multiple escapes in one string",
            decode("\\u003cscript\\u003e\\u0026\\u003c/script\\u003e"),
            std::string("<script>&</script>"));

        out.emplace_back("String with no escapes unchanged",
            decode("Hello World"), std::string("Hello World"));

        return {"Unicode Escape Decoding", out};
    }

} // dropout_dl

std::vector<dropout_dl::tests> test_season() {
    return {
        dropout_dl::test_episode_list_parsing(),
        dropout_dl::test_decode_unicode_escapes(),
    };
}
