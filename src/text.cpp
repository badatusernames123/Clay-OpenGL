#include "text.h"

#include <vector>
#include <string>
#include <cstdint>

#include "gl_util.h"



int create_character_atlas(CharacterAtlas* atlas, std::string font_filepath, uint16_t font_size)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_filepath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load " + font_filepath << std::endl;
        return -1;
    }

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE)) {
        std::cout << "ERROR::FREETYPE: Failed to load " + font_filepath << std::endl;
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    float metric_scale = 1.0f / 64.0f;
    atlas->ascender = std::max(0.0f, face->size->metrics.ascender * metric_scale);
    atlas->descender = std::max(0.0f, -face->size->metrics.descender * metric_scale);
    atlas->line_height = face->size->metrics.height * metric_scale;
    if (atlas->line_height <= 0.0f)
    {
        atlas->line_height = atlas->ascender + atlas->descender;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Limit to ASCII 0-127 for now to avoid oversized textures
    const uint32_t max_codepoint = 127;
    atlas->characters.resize(max_codepoint + 1);

    // Calculate total width and height needed for texture
    uint32_t texture_height = 0;
    uint32_t texture_width = 0;
    for (uint32_t charcode = 0; charcode <= max_codepoint; ++charcode) {
        if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
            continue;
        }

        uint32_t bitmap_width = face->glyph->bitmap.width;
        uint32_t bitmap_height = face->glyph->bitmap.rows;
        uint32_t padded_width = (bitmap_width > 0 ? bitmap_width + 2 : 2); // border on left/right
        texture_height += (bitmap_height > 0 ? bitmap_height + 2 : 2); // glyph pixels plus top/bottom borders
        texture_width = std::max(padded_width, texture_width);
    }

    if (texture_width == 0) {
        texture_width = 2;
    }
    if (texture_height == 0) {
        texture_height = 2;
    }

    // Diagnostic: Query and log GL_MAX_TEXTURE_SIZE
    GLint gl_max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
    // std::cout << "Font: " << font_filepath << ", Size: " << font_size
    //           << ", Calculated texture: " << texture_width << "x" << texture_height
    //           << ", GL_MAX_TEXTURE_SIZE: " << gl_max_texture_size << std::endl;

    if (texture_width == 0 || texture_height == 0 || texture_width > static_cast<uint32_t>(gl_max_texture_size) || texture_height > static_cast<uint32_t>(gl_max_texture_size)) {
        std::cout << "ERROR: Invalid texture dimensions for atlas" << std::endl;
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return -1;
    }

    // Fill texture
    std::vector<uint8_t> texture(texture_width * texture_height, 0);
    uint32_t current_row = 0;
    for (uint32_t charcode = 0; charcode <= max_codepoint; ++charcode) {
        if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
            continue;
        }

        uint32_t bitmap_width = face->glyph->bitmap.width;
        uint32_t bitmap_height = face->glyph->bitmap.rows;

        Character character;
        character.codepoint = charcode;
        character.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        character.advance = face->glyph->advance.x;
        character.size = { bitmap_width, bitmap_height };

        uint32_t block_height = (bitmap_height > 0 ? bitmap_height + 2 : 2);
        uint32_t row_top = current_row;
        if (bitmap_width == 0 || bitmap_height == 0)
        {
            character.bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
        else
        {
            character.bounds = {
                1.0f / texture_width,
                (float)(1 + bitmap_width) / texture_width,
                (float)(row_top + 1) / texture_height,
                (float)(row_top + 1 + bitmap_height) / texture_height,
            };

            const unsigned char* glyph_buffer = face->glyph->bitmap.buffer;

            for (uint32_t row = 0; row < bitmap_height; ++row) {
                const unsigned char* src_row = glyph_buffer + row * bitmap_width;
                uint8_t* dest = texture.data() + (row_top + 1 + row) * texture_width + 1;
                memcpy(dest, src_row, bitmap_width);

                dest[-1] = src_row[0];
                dest[bitmap_width] = src_row[bitmap_width - 1];
            }

            const unsigned char* first_row = glyph_buffer;
            const unsigned char* last_row = glyph_buffer + (bitmap_height - 1) * bitmap_width;
            uint8_t* top_row = texture.data() + row_top * texture_width + 1;
            uint8_t* bottom_row = texture.data() + (row_top + bitmap_height + 1) * texture_width + 1;
            memcpy(top_row, first_row, bitmap_width);
            memcpy(bottom_row, last_row, bitmap_width);

            top_row[-1] = first_row[0];
            top_row[bitmap_width] = first_row[bitmap_width - 1];
            bottom_row[-1] = last_row[0];
            bottom_row[bitmap_width] = last_row[bitmap_width - 1];
        }

        atlas->characters[charcode] = character;

        current_row += block_height;
    }
    
    // Save to file
    std::string png_filepath = font_filepath + "_" + std::to_string(font_size) + ".png";
    // stbi_write_png(png_filepath.c_str(), texture_width, texture_height, 1, texture.data(), texture_width);

    // Upload texture
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        texture_width,
        texture_height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        texture.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    atlas->texture_id = texture_id;

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    checkOpenGLErrors("Character atlas creation");

    return 0;
}



void utf8_to_codepoints(const std::string& text, std::vector<uint32_t>& codepoints) {
    size_t i = 0;
    while (i < text.length()) {
        unsigned char byte = static_cast<unsigned char>(text[i]);
        uint32_t codepoint = 0;
        int bytes_consumed = 0;

        if (byte < 0x80) {
            // Single-byte (ASCII)
            codepoint = byte;
            bytes_consumed = 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // Two-byte sequence
            if (i + 1 >= text.length()) break;
            unsigned char byte2 = static_cast<unsigned char>(text[i + 1]);
            if ((byte2 & 0xC0) != 0x80) { ++i; continue; } // Invalid continuation
            codepoint = ((byte & 0x1F) << 6) | (byte2 & 0x3F);
            bytes_consumed = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // Three-byte sequence
            if (i + 2 >= text.length()) break;
            unsigned char byte2 = static_cast<unsigned char>(text[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(text[i + 2]);
            if ((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80) { ++i; continue; } // Invalid
            codepoint = ((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
            bytes_consumed = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // Four-byte sequence
            if (i + 3 >= text.length()) break;
            unsigned char byte2 = static_cast<unsigned char>(text[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(text[i + 2]);
            unsigned char byte4 = static_cast<unsigned char>(text[i + 3]);
            if ((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80 || (byte4 & 0xC0) != 0x80) { ++i; continue; } // Invalid
            codepoint = ((byte & 0x07) << 18) | ((byte2 & 0x3F) << 12) | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
            bytes_consumed = 4;
        } else {
            // Invalid leading byte
            ++i;
            continue;
        }

        i += bytes_consumed;

        // Optional: Validate codepoint range (e.g., skip surrogates or invalid)
        if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            continue;
        }

        codepoints.push_back(codepoint);
    }
}














