#include "stringTranslator.hpp"
#include <cassert>

// Generate the English Dictionary
#define GENERATE_EN(KEY, EN, HU) EN,
static const char* const dict_EN[] = {
    TRANSLATION_TABLE(GENERATE_EN)
};
#undef GENERATE_EN

// Generate the hun Dictionary
#define GENERATE_HU(KEY, EN, HU) HU,
static const char* const dict_HU[] = {
    TRANSLATION_TABLE(GENERATE_HU)
};
#undef GENERATE_HU

static const char* const* dictionaries[] = { dict_EN, dict_HU };

int Translator::currentLang = 0;

void Translator::SetLanguage(int langId) {
    assert(langId >= 0 && langId < 2 && "Translator: Invalid language ID");
    currentLang = langId;
}

const char* Translator::Get(const TextKey key) {
    assert(key < TextKey::Count && "Translator: Invalid TextKey");
    return dictionaries[currentLang][static_cast<int>(key)];
}