#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image_write.h"

#include "rect.h"


struct Character
{
    uint32_t codepoint;
    Rect bounds; // Bounds in the atlas
    glm::ivec2 bearing;
    glm::ivec2 size;
    unsigned int advance;
};

struct CharacterAtlas
{
    uint32_t texture_id;
    std::vector<Character> characters;
    float ascender;
    float descender;
    float line_height;
};

int create_character_atlas(CharacterAtlas* atlas, std::string font_filepath, uint16_t font_size);

void utf8_to_codepoints(const std::string& text, std::vector<uint32_t>& codepoints);


