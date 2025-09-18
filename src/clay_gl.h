#pragma once

#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>

#include "clay.h"

#include "text.h"
#include "rect.h"

struct CharacterVertex
{
    glm::vec2 pos;
    glm::vec2 uv;
};

struct ClayRectVertex
{
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 uv;

    void print()
    {
        std::cout 
        << "{\n" 
            << "\t{" << pos.x << ", " << pos.y << "}\n"
            << "\t{" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << "}\n"
            << "\t{" << uv.x << ", " << uv.y <<  "}\n"
        << "}\n";
    }
};

struct ClayRenderCtx
{
    std::vector<ClayRectVertex> rect_vertices;
    uint32_t rect_VAO;
    uint32_t rect_VBO;
    uint32_t rect_shader;

    std::vector<CharacterVertex> text_vertices;
    uint32_t text_VAO = 0;
    uint32_t text_VBO = 0;
    uint32_t text_shader;

    std::vector<glm::vec4> scissor_stack;

    std::unordered_map<std::string, uint32_t> texture_ids; // uint32_t texture_id = texture_ids[image_filepath];

    std::vector<std::string> fonts;
    std::map<std::string, std::map<uint16_t, CharacterAtlas>> character_atlases; // uint32_t atlas_id = character_atlases[font_filepath][font_size].texture_id;

    glm::mat4 projection;
};

std::string read_file(std::string filepath);

uint16_t get_font_id(ClayRenderCtx* ctx, std::string font);

uint32_t create_shader(std::string vertex_file, std::string fragment_file);

void clay_init_render_ctx(ClayRenderCtx* ctx, std::vector<std::string> image_filepaths, std::vector<std::string> font_filepaths);

void clay_render(Clay_RenderCommandArray commands, ClayRenderCtx* ctx, int window_width, int window_height);

void draw_clay_text_debug(ClayRenderCtx* ctx, Clay_RenderCommand command);

Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* user_data);
