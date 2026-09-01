#include "stringTranslator.hpp"
#include <cassert>

// 1. Generate the English Dictionary
#define GENERATE_EN(KEY, EN, HU) EN,
static const char* const dict_EN[] = {
    TRANSLATION_TABLE(GENERATE_EN)
};
#undef GENERATE_EN

// 2. Generate the Spanish (Hungarian) Dictionary
#define GENERATE_HU(KEY, EN, HU) HU,
static const char* const dict_HU[] = {
    TRANSLATION_TABLE(GENERATE_HU)
};
#undef GENERATE_HU

// 3. Create the Master Dictionary Array
static const char* const* dictionaries[] = { dict_EN, dict_HU };

// 4. Define the static member variable
int Translator::currentLang = 0;

// 5. Implement the Methods
void Translator::setLanguage(int langId) {
    // Basic bounds checking for safety
    assert(langId >= 0 && langId < 2 && "Translator: Invalid language ID");
    currentLang = langId;
}

const char* Translator::get(const TextKey key) {
    // Prevent out-of-bounds array access
    assert(key < TextKey::Count && "Translator: Invalid TextKey");
    return dictionaries[currentLang][static_cast<int>(key)];
}