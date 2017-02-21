#include "catch.hpp"
#include <mapnik/text/icu_shaper.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/font_library.hpp>
#include <mapnik/unicode.hpp>

namespace {

void test_shaping( mapnik::font_set const& fontset, mapnik::face_manager& fm,
                  std::vector<std::tuple<unsigned, unsigned>> const& expected, char const* str)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned,double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;

    double scale_factor = 1;
    auto ustr = tr.transcode(str);
    auto length = ustr.length();
    itemizer.add_text(ustr, props);

    mapnik::text_line line(0, length);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer,
                                        width_map,
                                        fm,
                                        scale_factor);

    std::size_t index = 0;
    for (auto const& g : line)
    {
        unsigned glyph_index, char_index;
        CHECK(index < expected.size());
        std::tie(glyph_index, char_index) = expected[index++];
        REQUIRE(glyph_index == g.glyph_index);
        REQUIRE(char_index == g.char_index);
    }
}
}

TEST_CASE("shaping")
{
    mapnik::freetype_engine::register_font("test/data/fonts/NotoSans-Regular.ttc");
    mapnik::freetype_engine::register_fonts("test/data/fonts/Noto");
    mapnik::font_set fontset("fontset");
    for (auto const& name : mapnik::freetype_engine::face_names())
    {
        fontset.add_face_name(name);
    }

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    {
        std::vector<std::tuple<unsigned, unsigned>> expected =
            {{0, 0}, {0, 3}, {0, 4}, {0, 7}, {3, 8}, {11, 9}, {68, 10}, {69, 11}, {70, 12}, {12, 13}};
        // with default NotoSans-Regular.ttc and NotoNaskhArabic-Regular.ttf ^^^
        //std::vector<std::tuple<unsigned, unsigned>> expected =
        //  {{977,0}, {1094,3}, {1038,4}, {1168,4}, {9,7}, {3,8}, {11,9}, {68,10}, {69,11}, {70,12}, {12,13}};
        // expected results if "NotoSansTibetan-Regular.ttf is registered^^
        test_shaping(fontset, fm, expected, "སྤུ་ཧྲེང (abc)");
    }

    {
        std::vector<std::tuple<unsigned, unsigned>> expected =
            {{0, 0}, {0, 3}, {0, 4}, {0, 7}, {3, 8}, {11, 9}, {0, 10}, {0, 11}, {0, 12}, {12, 13}};
        test_shaping(fontset, fm, expected, "སྤུ་ཧྲེང (普兰镇)");
    }

}
