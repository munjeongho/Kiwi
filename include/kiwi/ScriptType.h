#pragma once
#include <cstdint>

namespace kiwi
{
    enum class ScriptType : uint8_t 
    {
        unknown,
        latin,
        ipa_extensions,
        spacing_modifier_letters,
        combining_diacritical_marks,
        greek_and_coptic,
        cyrillic,
        armenian,
        hebrew,
        arabic,
        syriac,
        thaana,
        nko,
        samaritan,
        mandaic,
        devanagari,
        bengali,
        gurmukhi,
        gujarati,
        oriya,
        tamil,
        telugu,
        kannada,
        malayalam,
        sinhala,
        thai,
        lao,
        tibetan,
        myanmar,
        georgian,
        hangul,
        ethiopic,
        cherokee,
        unified_canadian_aboriginal_syllabics,
        ogham,
        runic,
        tagalog,
        hanunoo,
        buhid,
        tagbanwa,
        khmer,
        mongolian,
        limbu,
        tai_le,
        new_tai_lue,
        khmer_symbols,
        buginese,
        tai_tham,
        balinese,
        sundanese,
        batak,
        lepcha,
        ol_chiki,
        phonetic_extensions,
        punctuation,
        superscripts_and_subscripts,
        currency_symbols,
        combining_diacritical_marks_for_symbols,
        letterlike_symbols,
        number_forms,
        arrows,
        mathematical,
        miscellaneous_technical,
        control_pictures,
        optical_character_recognition,
        enclosed_alphanumerics,
        box_drawing,
        block_elements,
        geometric_shapes,
        miscellaneous_symbols,
        dingbats,
        braille_patterns,
        glagolitic,
        tifinagh,
        hanja,
        ideographic_description_characters,
        kana,
        bopomofo,
        kanbun,
        yijing_hexagram_symbols,
        yi,
        lisu,
        vai,
        bamum,
        modifier_tone_letters,
        syloti_nagri,
        common_indic_number_forms,
        phags_pa,
        saurashtra,
        kayah_li,
        rejang,
        javanese,
        cham,
        tai_viet,
        meetei_mayek,
        private_use_area,
        alphabetic_presentation_forms,
        arabic_presentation_forms_a,
        variation_selectors,
        vertical_forms,
        combining_half_marks,
        small_form_variants,
        arabic_presentation_forms_b,
        halfwidth_and_fullwidth_forms,
        specials,
        linear_b,
        aegean_numbers,
        ancient_greek_numbers,
        ancient_symbols,
        phaistos_disc,
        lycian,
        carian,
        coptic_epact_numbers,
        old_italic,
        gothic,
        old_permic,
        ugaritic,
        old_persian,
        deseret,
        shavian,
        osmanya,
        osage,
        elbasan,
        caucasian_albanian,
        vithkuqi,
        linear_a,
        cypriot_syllabary,
        imperial_aramaic,
        palmyrene,
        nabataean,
        hatran,
        phoenician,
        lydian,
        meroitic_hieroglyphs,
        meroitic_cursive,
        kharoshthi,
        old_south_arabian,
        old_north_arabian,
        manichaean,
        avestan,
        inscriptional_parthian,
        inscriptional_pahlavi,
        psalter_pahlavi,
        old_turkic,
        old_hungarian,
        hanifi_rohingya,
        rumi_numeral_symbols,
        yezidi,
        old_sogdian,
        sogdian,
        old_uyghur,
        chorasmian,
        elymaic,
        brahmi,
        kaithi,
        sora_sompeng,
        chakma,
        mahajani,
        sharada,
        sinhala_archaic_numbers,
        khojki,
        multani,
        khudawadi,
        grantha,
        newa,
        tirhuta,
        siddham,
        modi,
        takri,
        ahom,
        dogra,
        warang_citi,
        dives_akuru,
        nandinagari,
        zanabazar_square,
        soyombo,
        pau_cin_hau,
        bhaiksuki,
        marchen,
        masaram_gondi,
        gunjala_gondi,
        makasar,
        kawi,
        cuneiform,
        early_dynastic_cuneiform,
        cypro_minoan,
        egyptian_hieroglyphs,
        anatolian_hieroglyphs,
        mro,
        tangsa,
        bassa_vah,
        pahawh_hmong,
        medefaidrin,
        miao,
        ideographic_symbols_and_punctuation,
        tangut,
        khitan_small_script,
        nushu,
        duployan,
        shorthand_format_controls,
        znamenny_musical_notation,
        byzantine_musical_symbols,
        musical_symbols,
        ancient_greek_musical_notation,
        kaktovik_numerals,
        mayan_numerals,
        tai_xuan_jing_symbols,
        counting_rod_numerals,
        mathematical_alphanumeric_symbols,
        sutton_signwriting,
        nyiakeng_puachue_hmong,
        toto,
        wancho,
        nag_mundari,
        mende_kikakui,
        adlam,
        indic_siyaq_numbers,
        ottoman_siyaq_numbers,
        arabic_mathematical_alphabetic_symbols,
        mahjong_tiles,
        domino_tiles,
        playing_cards,
        enclosed_ideographic_supplement,
        symbols_and_pictographs,
        emoticons,
        transport_and_map_symbols,
        alchemical_symbols,
        chess_symbols,
        symbols_for_legacy_computing,
        tags,
        max,
    };

    ScriptType chr2ScriptType(char32_t c);

    const char* getScriptName(ScriptType type);

    /**
     * @brief Check if the character is an emoji
     * 
     * @return 0 if the character is not an emoji, 1 if c0 is an emoji, 2 if c0 and c1 are combined to form an emoji.
     */
    int isEmoji(char32_t c0, char32_t c1 = 0);
}
