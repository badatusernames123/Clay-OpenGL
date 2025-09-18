#include "clay_gl.h"

#include <stack>
#include <algorithm>
#include <limits>

#include "stb_image.h"

#include "gl_util.h"

std::string read_file(std::string filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) 
    {
        std::cout << "Failed to open file: " << filepath << std::endl;
        exit(1);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}

uint32_t create_shader(std::string vertex_file, std::string fragment_file)
{
    std::string vertex_source = read_file(vertex_file);
    std::string fragment_source = read_file(fragment_file);
    const char* vert_str = vertex_source.c_str();
    const char* frag_str = fragment_source.c_str();

    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vert_str, nullptr);
    glShaderSource(fragment_shader, 1, &frag_str, nullptr);

    glCompileShader(vertex_shader);
    // Check compilation status
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
        std::cout << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog);
        std::cout << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    uint32_t id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) 
    {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cout << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    return id;
}



void clay_init_render_ctx(ClayRenderCtx* ctx, std::vector<std::string> image_filepaths, std::vector<std::string> font_filepaths)
{
    ctx->text_shader = create_shader("src/shaders/text.vert", "src/shaders/text.frag");
    glGenVertexArrays(1, &ctx->text_VAO);
    glGenBuffers(1, &ctx->text_VBO);
    glBindVertexArray(ctx->text_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->text_VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    ctx->rect_shader = create_shader("src/shaders/rect.vert", "src/shaders/rect.frag");
    glGenVertexArrays(1, &ctx->rect_VAO);
    glGenBuffers(1, &ctx->rect_VBO);
    glBindVertexArray(ctx->rect_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->rect_VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    for (const auto& filepath : image_filepaths) 
    {
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
        if (!data) 
        {
            std::cout << "Failed to load image: " << filepath << std::endl;
            continue;
        }

        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        ctx->texture_ids[filepath] = texture;
    }

    unsigned char white_data[] = {255, 255, 255, 255};
    uint32_t white_texture;
    glGenTextures(1, &white_texture);
    glBindTexture(GL_TEXTURE_2D, white_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    ctx->texture_ids["white"] = white_texture;
    
    uint16_t font_sizes[] = { 12, 14, 16, 20, 24, 32, 44, 64 };
    for (uint32_t i = 0; i < font_filepaths.size(); i++)
    {
        std::string filepath = font_filepaths[i];
        for (const auto& font_size : font_sizes)
        {
            CharacterAtlas atlas;
            create_character_atlas(&atlas, filepath, font_size);
            ctx->character_atlases[filepath][font_size] = atlas;
            ctx->fonts.push_back(filepath);
        }
    }

    Clay_SetMeasureTextFunction(MeasureText, ctx);
}

glm::vec4 normalize_clay_color(Clay_Color color)
{
    return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
}

struct Quad
{
    glm::vec2 v0;
    glm::vec2 v1;
    glm::vec2 v2;
    glm::vec2 v3;
};
struct Arc
{
    glm::vec2 pos;
    float radius;
    float width_start;
    float width_end;
    glm::vec4 color;
    float start;
    float end;
};

const float PI = 3.14159f;

void add_rect_vertex(ClayRenderCtx* ctx, glm::vec2 pos, glm::vec4 color, Rect bb)
{
    ctx->rect_vertices.push_back({
        pos,
        color,
        { (pos.x - bb.left) / (bb.right - bb.left), (pos.y - bb.top) / (bb.bot - bb.top) }
    });
}

void add_quad(ClayRenderCtx* ctx, glm::vec4 color, Quad quad, Rect bb)
{
    add_rect_vertex(ctx, quad.v0, color, bb);
    add_rect_vertex(ctx, quad.v1, color, bb);
    add_rect_vertex(ctx, quad.v2, color, bb);

    add_rect_vertex(ctx, quad.v0, color, bb);
    add_rect_vertex(ctx, quad.v2, color, bb);
    add_rect_vertex(ctx, quad.v3, color, bb);
}

void add_corner(ClayRenderCtx* ctx, glm::vec2 pos, glm::vec4 color, uint32_t radius, float start, float end, Rect bb)
{
    if (radius == 0) return;

    const int segments = 16;
    float angle_step = (end - start) / static_cast<float>(segments);

    glm::vec2 prev_pos = pos + static_cast<float>(radius) * glm::vec2(std::cos(start), std::sin(start));

    for (int i = 1; i <= segments; ++i) 
    {
        float angle = start + static_cast<float>(i) * angle_step;
        glm::vec2 curr_pos = pos + static_cast<float>(radius) * glm::vec2(std::cos(angle), std::sin(angle));

        add_rect_vertex(ctx, pos, color, bb);
        add_rect_vertex(ctx, prev_pos, color, bb);
        add_rect_vertex(ctx, curr_pos, color, bb);

        prev_pos = curr_pos;
    }
}

void add_arc(ClayRenderCtx* ctx, Arc arc)
{
    const int segments = 16;
    float angle_step = (arc.end - arc.start) / static_cast<float>(segments);
    float width_step = (arc.width_end - arc.width_start) / static_cast<float>(segments);

    glm::vec2 prev_inner = arc.pos + arc.radius * glm::vec2(std::cos(arc.start), std::sin(arc.start));
    float curr_width = arc.width_start;
    glm::vec2 prev_outer = arc.pos + (arc.radius + curr_width) * glm::vec2(std::cos(arc.start), std::sin(arc.start));

    Rect dummy_bb = {0.0f, 1.0f, 0.0f, 1.0f};

    for (int i = 1; i <= segments; ++i) 
    {
        float angle = arc.start + static_cast<float>(i) * angle_step;
        curr_width = arc.width_start + static_cast<float>(i) * width_step;
        glm::vec2 curr_inner = arc.pos + arc.radius * glm::vec2(std::cos(angle), std::sin(angle));
        glm::vec2 curr_outer = arc.pos + (arc.radius + curr_width) * glm::vec2(std::cos(angle), std::sin(angle));

        add_rect_vertex(ctx, prev_inner, arc.color, dummy_bb);
        add_rect_vertex(ctx, prev_outer, arc.color, dummy_bb);
        add_rect_vertex(ctx, curr_inner, arc.color, dummy_bb);

        add_rect_vertex(ctx, curr_inner, arc.color, dummy_bb);
        add_rect_vertex(ctx, prev_outer, arc.color, dummy_bb);
        add_rect_vertex(ctx, curr_outer, arc.color, dummy_bb);

        prev_inner = curr_inner;
        prev_outer = curr_outer;
    }
}


void draw_clay_rectangle(ClayRenderCtx* ctx, Clay_RenderCommand command)
{  
    glm::vec4 color;
    Clay_CornerRadius cr;
    uint32_t texture_id;
    if (command.commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE) 
    {
        color = normalize_clay_color(command.renderData.image.backgroundColor);
        cr = command.renderData.image.cornerRadius;
        texture_id = ctx->texture_ids[(char*)command.renderData.image.imageData];
    }
    else
    {
        color = normalize_clay_color(command.renderData.rectangle.backgroundColor);
        cr = command.renderData.rectangle.cornerRadius;
        texture_id = ctx->texture_ids["white"];
    }
    
    Rect bb = { 
        command.boundingBox.x, 
        command.boundingBox.x + command.boundingBox.width, 
        command.boundingBox.y, 
        command.boundingBox.y + command.boundingBox.height,
    };

    // V0 - V3
    // |    |
    // V1 - V2

    Quad top;
    top.v0 = { bb.left + cr.topLeft, bb.top };
    top.v1 = { bb.left + cr.topLeft, bb.top + cr.topLeft };
    top.v2 = { bb.right - cr.topRight, bb.top + cr.topRight };
    top.v3 = { bb.right - cr.topRight, bb.top };
    add_quad(ctx, color, top, bb);

    Quad center;
    center.v0 = { bb.left + cr.topLeft, bb.top + cr.topLeft };
    center.v1 = { bb.left + cr.bottomLeft, bb.bot - cr.bottomLeft };
    center.v2 = { bb.right - cr.bottomRight, bb.bot - cr.bottomRight };
    center.v3 = { bb.right - cr.topRight, bb.top + cr.topRight };
    add_quad(ctx, color, center, bb);

    Quad bot;
    bot.v0 = { bb.left + cr.bottomLeft, bb.bot - cr.bottomLeft };
    bot.v1 = { bb.left + cr.bottomLeft, bb.bot };
    bot.v2 = { bb.right - cr.bottomRight, bb.bot };
    bot.v3 = { bb.right - cr.bottomRight, bb.bot - cr.bottomRight };
    add_quad(ctx, color, bot, bb);

    Quad left;
    left.v0 = { bb.left, bb.top + cr.topLeft };
    left.v1 = { bb.left, bb.bot - cr.bottomLeft };
    left.v2 = { bb.left + cr.bottomLeft, bb.bot - cr.bottomLeft };
    left.v3 = { bb.left + cr.topLeft, bb.top + cr.topLeft };
    add_quad(ctx, color, left, bb);
    
    Quad right;
    right.v0 = { bb.right - cr.topRight, bb.top + cr.topRight };
    right.v1 = { bb.right - cr.bottomRight, bb.bot - cr.bottomRight };
    right.v2 = { bb.right, bb.bot - cr.bottomRight };
    right.v3 = { bb.right, bb.top + cr.topRight };
    add_quad(ctx, color, right, bb);

    // Top left corner
    glm::vec2 corner_pos = { bb.left + cr.topLeft, bb.top + cr.topLeft };
    add_corner(ctx, corner_pos, color, cr.topLeft, PI, 3.0f * PI / 2.0f, bb);

    // Bottom left corner
    corner_pos = { bb.left + cr.bottomLeft, bb.bot - cr.bottomLeft };
    add_corner(ctx, corner_pos, color, cr.bottomLeft, PI / 2.0f, PI, bb);

    // Bottom right corner
    corner_pos = { bb.right - cr.bottomRight, bb.bot - cr.bottomRight };
    add_corner(ctx, corner_pos, color, cr.bottomRight, 0.0f, PI / 2.0f, bb);

    // Top right corner
    corner_pos = { bb.right - cr.topRight, bb.top + cr.topRight };
    add_corner(ctx, corner_pos, color, cr.topRight, 3.0f * PI / 2.0f, 2.0f * PI, bb);

    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(ctx->rect_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->rect_VBO);
    glBufferData(GL_ARRAY_BUFFER, ctx->rect_vertices.size() * sizeof(ClayRectVertex), ctx->rect_vertices.data(), GL_STREAM_DRAW);
    
    glUseProgram(ctx->rect_shader);
    glUniformMatrix4fv(glGetUniformLocation(ctx->rect_shader, "projection"), 1, GL_FALSE, glm::value_ptr(ctx->projection));
    glUniform1i(glGetUniformLocation(ctx->rect_shader, "tex_sampler"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glDrawArrays(GL_TRIANGLES, 0, ctx->rect_vertices.size());
    
    checkOpenGLErrors("Rectangle draw");
}

void draw_clay_border(ClayRenderCtx* ctx, Clay_RenderCommand command)
{
    glm::vec4 color = normalize_clay_color(command.renderData.border.color);
    Clay_CornerRadius cr = command.renderData.border.cornerRadius;
    Clay_BorderWidth bw = command.renderData.border.width;
    uint32_t texture_id = ctx->texture_ids["white"];
    
    Rect bb = { 
        command.boundingBox.x, 
        command.boundingBox.x + command.boundingBox.width, 
        command.boundingBox.y, 
        command.boundingBox.y + command.boundingBox.height,
    };

    Rect dummy_bb = {0.f, 1.f, 0.f, 1.f};  // Dummy to avoid div-zero in UV calc; fine for white texture

    // Left
    Quad left;
    left.v0 = { bb.left - bw.left, bb.top + cr.topLeft };
    left.v1 = { bb.left - bw.left, bb.bot - cr.bottomLeft };
    left.v2 = { bb.left, bb.bot - cr.bottomLeft };
    left.v3 = { bb.left, bb.top + cr.topLeft };
    add_quad(ctx, color, left, dummy_bb);

    // Right
    Quad right;
    right.v0 = { bb.right + bw.right, bb.top + cr.topRight };
    right.v1 = { bb.right + bw.right, bb.bot - cr.bottomRight };
    right.v2 = { bb.right, bb.bot - cr.bottomRight };
    right.v3 = { bb.right, bb.top + cr.topRight };
    add_quad(ctx, color, right, dummy_bb);

    // Top
    Quad top;
    top.v0 = { bb.left + cr.topLeft, bb.top - bw.top };
    top.v1 = { bb.left + cr.topLeft, bb.top };
    top.v2 = { bb.right - cr.topRight, bb.top };
    top.v3 = { bb.right - cr.topRight, bb.top - bw.top };
    add_quad(ctx, color, top, dummy_bb);

    // Bottom
    Quad bot;
    bot.v0 = { bb.left + cr.bottomLeft, bb.bot + bw.bottom };
    bot.v1 = { bb.left + cr.bottomLeft, bb.bot };
    bot.v2 = { bb.right - cr.bottomRight, bb.bot };
    bot.v3 = { bb.right - cr.bottomRight, bb.bot + bw.bottom };
    add_quad(ctx, color, bot, dummy_bb);

    // Top left
    Arc top_left;
    top_left.pos = { bb.left + cr.topLeft, bb.top + cr.topLeft };
    top_left.radius = cr.topLeft;
    top_left.width_start = bw.left;
    top_left.width_end   = bw.top;
    top_left.color = color;
    top_left.start = PI;
    top_left.end   = 3.0f * PI / 2.0f;
    add_arc(ctx, top_left);

    // Bottom left
    Arc bottom_left;
    bottom_left.pos = { bb.left + cr.bottomLeft, bb.bot  - cr.bottomLeft };
    bottom_left.radius = cr.bottomLeft;
    bottom_left.width_start = bw.bottom;
    bottom_left.width_end   = bw.left;
    bottom_left.color = color;
    bottom_left.start = PI / 2.0f;
    bottom_left.end   = PI;
    add_arc(ctx, bottom_left);

    // Bottom right
    Arc bottom_right;
    bottom_right.pos = { bb.right - cr.bottomRight, bb.bot  - cr.bottomRight };
    bottom_right.radius = cr.bottomRight;
    bottom_right.width_start = bw.right;
    bottom_right.width_end   = bw.bottom;
    bottom_right.color = color;
    bottom_right.start = 0.0f;
    bottom_right.end   = PI / 2.0f;
    add_arc(ctx, bottom_right);

    // Top right
    Arc top_right;
    top_right.pos = { bb.right - cr.topRight,    bb.top + cr.topRight };
    top_right.radius = cr.topRight;
    top_right.width_start = bw.top;
    top_right.width_end   = bw.right;
    top_right.color = color;
    top_right.start = 3.0f * PI / 2.0f;
    top_right.end   = 2.0f * PI;
    add_arc(ctx, top_right);

    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(ctx->rect_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->rect_VBO);
    glBufferData(GL_ARRAY_BUFFER, ctx->rect_vertices.size() * sizeof(ClayRectVertex), ctx->rect_vertices.data(), GL_STREAM_DRAW);
    
    glUseProgram(ctx->rect_shader);
    glUniformMatrix4fv(glGetUniformLocation(ctx->rect_shader, "projection"), 1, GL_FALSE, glm::value_ptr(ctx->projection));
    glUniform1i(glGetUniformLocation(ctx->rect_shader, "tex_sampler"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glDrawArrays(GL_TRIANGLES, 0, ctx->rect_vertices.size());
    
    checkOpenGLErrors("Border draw");
}

void draw_clay_text(ClayRenderCtx* ctx, Clay_RenderCommand command)
{
    uint16_t font_id = command.renderData.text.fontId;
    uint16_t font_size = command.renderData.text.fontSize;
    glm::vec4 color = normalize_clay_color(command.renderData.text.textColor);
    std::string text(command.renderData.text.stringContents.chars, command.renderData.text.stringContents.length);

    CharacterAtlas* atlas = &ctx->character_atlases[ctx->fonts[font_id]][font_size];

    Rect bb = { 
        command.boundingBox.x, 
        command.boundingBox.x + command.boundingBox.width, 
        command.boundingBox.y, 
        command.boundingBox.y + command.boundingBox.height,
    };

    std::vector<uint32_t> codepoints;
    utf8_to_codepoints(text, codepoints);

    float ascender = atlas->ascender;
    float descender = atlas->descender;
    if (ascender <= 0.0f && descender <= 0.0f)
    {
        ascender = static_cast<float>(font_size);
        descender = 0.0f;
    }

    float text_height = ascender + descender;
    float layout_height = bb.bot - bb.top;
    float vertical_extra = std::max(0.0f, layout_height - text_height);
    float baseline = bb.top + vertical_extra * 0.5f + ascender;

    float x = bb.left;
    float y = baseline;
    for (int i = 0; i < codepoints.size(); i++)
    {
        if (codepoints[i] >= atlas->characters.size()) continue;
        Character ch = atlas->characters[codepoints[i]];

        float xpos = x + ch.bearing.x;
        float ypos = y + (ch.size.y - ch.bearing.y);

        float w = ch.size.x;
        float h = ch.size.y;

        // Create vertices
        CharacterVertex vertex;

        // top left
        vertex.pos = { xpos, ypos - h };
        vertex.uv = { ch.bounds.left, ch.bounds.top };
        ctx->text_vertices.push_back(vertex);

        // bottom left
        vertex.pos = { xpos, ypos };
        vertex.uv = { ch.bounds.left, ch.bounds.bot };
        ctx->text_vertices.push_back(vertex);

        // bottom right
        vertex.pos = { xpos + w, ypos };
        vertex.uv = { ch.bounds.right, ch.bounds.bot };
        ctx->text_vertices.push_back(vertex);

        // top left
        vertex.pos = { xpos, ypos - h };
        vertex.uv = { ch.bounds.left, ch.bounds.top };
        ctx->text_vertices.push_back(vertex);

        // bottom right
        vertex.pos = { xpos + w, ypos };
        vertex.uv = { ch.bounds.right, ch.bounds.bot };
        ctx->text_vertices.push_back(vertex);

        // top right
        vertex.pos = { xpos + w, ypos - h };
        vertex.uv = { ch.bounds.right, ch.bounds.top };
        ctx->text_vertices.push_back(vertex);

        x += ch.advance >> 6;

    }
    glUseProgram(ctx->text_shader);
    glUniformMatrix4fv(glGetUniformLocation(ctx->text_shader, "projection"), 1, GL_FALSE, glm::value_ptr(ctx->projection));
    glUniform1i(glGetUniformLocation(ctx->text_shader, "character_atlas"), 0);
    glUniform4fv(glGetUniformLocation(ctx->text_shader, "text_color"), 1, glm::value_ptr(color));

    glBindVertexArray(ctx->text_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->text_VBO);
    glBufferData(GL_ARRAY_BUFFER, ctx->text_vertices.size() * sizeof(CharacterVertex), ctx->text_vertices.data(), GL_STREAM_DRAW);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);  
    
    glDrawArrays(GL_TRIANGLES, 0, ctx->text_vertices.size());
    
    checkOpenGLErrors("Text draw");
}
void draw_clay_text_debug(ClayRenderCtx* ctx, Clay_RenderCommand command)
{
    ctx->rect_vertices.clear();

    uint16_t font_id = command.renderData.text.fontId;
    uint16_t font_size = command.renderData.text.fontSize;

    Clay_TextElementConfig config = {};
    config.fontId = font_id;
    config.fontSize = font_size;
    config.letterSpacing = command.renderData.text.letterSpacing;
    config.lineHeight = command.renderData.text.lineHeight;

    Clay_Dimensions dims = MeasureText(command.renderData.text.stringContents, &config, ctx);
    if (dims.width <= 0.0f || dims.height <= 0.0f)
    {
        return;
    }

    CharacterAtlas* atlas = &ctx->character_atlases[ctx->fonts[font_id]][font_size];

    glm::vec4 color = normalize_clay_color(command.renderData.text.textColor);

    Rect layout_bb = {
        command.boundingBox.x,
        command.boundingBox.x + command.boundingBox.width,
        command.boundingBox.y,
        command.boundingBox.y + command.boundingBox.height,
    };

    float ascender = atlas->ascender;
    float descender = atlas->descender;
    if (ascender <= 0.0f && descender <= 0.0f)
    {
        ascender = static_cast<float>(font_size);
        descender = 0.0f;
    }

    float text_height = ascender + descender;
    float layout_height = layout_bb.bot - layout_bb.top;
    float vertical_extra = std::max(0.0f, layout_height - text_height);
    float baseline = layout_bb.top + vertical_extra * 0.5f + ascender;

    float top = baseline - ascender;
    float bottom = baseline + descender;

    Rect bb = { layout_bb.left, layout_bb.left + dims.width, top, bottom };

    Quad quad;
    quad.v0 = { bb.left, bb.top };
    quad.v1 = { bb.left, bb.bot };
    quad.v2 = { bb.right, bb.bot };
    quad.v3 = { bb.right, bb.top };
    add_quad(ctx, color, quad, bb);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(ctx->rect_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->rect_VBO);
    glBufferData(GL_ARRAY_BUFFER, ctx->rect_vertices.size() * sizeof(ClayRectVertex), ctx->rect_vertices.data(), GL_STREAM_DRAW);

    glUseProgram(ctx->rect_shader);
    glUniformMatrix4fv(glGetUniformLocation(ctx->rect_shader, "projection"), 1, GL_FALSE, glm::value_ptr(ctx->projection));
    glUniform1i(glGetUniformLocation(ctx->rect_shader, "tex_sampler"), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx->texture_ids["white"]);

    glDrawArrays(GL_TRIANGLES, 0, ctx->rect_vertices.size());

    checkOpenGLErrors("Text debug draw");
}


void clay_render(Clay_RenderCommandArray commands, ClayRenderCtx* ctx, int window_width, int window_height)
{
    ctx->projection = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f);

    std::stack<Rect> scissors;
    auto apply_scissor = [&](const Rect& r) {
        if (r.right <= r.left || r.bot <= r.top) {
            glDisable(GL_SCISSOR_TEST);
            return;
        }
        glEnable(GL_SCISSOR_TEST);
        GLint x = static_cast<GLint>(r.left);
        GLint width = static_cast<GLint>(r.right - r.left);
        GLint height = static_cast<GLint>(r.bot - r.top);
        GLint y = static_cast<GLint>(window_height - (r.top + height));
        glScissor(x, y, width, height);
    };
    scissors.push({ 0, (float)window_width, 0, (float)window_height });
    glDisable(GL_SCISSOR_TEST);

    for (int i = 0; i < commands.length; i++) 
    {
        ctx->rect_vertices.clear();
        ctx->text_vertices.clear();

        Clay_RenderCommand command = commands.internalArray[i];
        Rect bb = { 
            command.boundingBox.x, 
            command.boundingBox.x + command.boundingBox.width, 
            command.boundingBox.y, 
            command.boundingBox.y + command.boundingBox.height,
        };

        glm::vec4 color;
        Clay_CornerRadius corner_radius;
        uint32_t texture_id;

        switch (command.commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
                draw_clay_rectangle(ctx, command);
                break;
            case CLAY_RENDER_COMMAND_TYPE_BORDER:
                draw_clay_border(ctx, command);
                break;
            case CLAY_RENDER_COMMAND_TYPE_TEXT:
                draw_clay_text(ctx, command);
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                scissors.push(scissors.top().intersection(bb));
                apply_scissor(scissors.top());
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                if (scissors.size() > 1)
                {
                    scissors.pop();
                    if (scissors.size() > 1)
                    {
                        apply_scissor(scissors.top());
                    }
                    else
                    {
                        glDisable(GL_SCISSOR_TEST);
                    }
                }
                else
                {
                    glDisable(GL_SCISSOR_TEST);
                }
                break;
        }
    }
}

uint16_t get_font_id(ClayRenderCtx* ctx, std::string font)
{
    auto it = std::find(ctx->fonts.begin(), ctx->fonts.end(), font);
    if (it != ctx->fonts.end())
    {   
        return std::distance(ctx->fonts.begin(), it);
    } else
    {   
        return 0; // First font is default
    }
}

Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* user_data)
{
    Clay_Dimensions dims = { 0.0f, 0.0f };

    if (!user_data || !config || text.length == 0)
    {
        return dims;
    }

    ClayRenderCtx* ctx = static_cast<ClayRenderCtx*>(user_data);
    if (config->fontId >= ctx->fonts.size())
    {
        return dims;
    }

    const std::string& font_path = ctx->fonts[config->fontId];
    auto atlas_map_it = ctx->character_atlases.find(font_path);
    if (atlas_map_it == ctx->character_atlases.end())
    {
        return dims;
    }

    auto atlas_it = atlas_map_it->second.find(config->fontSize);
    if (atlas_it == atlas_map_it->second.end())
    {
        return dims;
    }

    const CharacterAtlas& atlas = atlas_it->second;
    if (atlas.characters.empty())
    {
        return dims;
    }

    std::string string(text.chars, text.length);
    std::vector<uint32_t> codepoints;
    codepoints.reserve(string.size());
    utf8_to_codepoints(string, codepoints);

    float pen_x = 0.0f;
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    bool has_geometry = false;

    for (uint32_t codepoint : codepoints)
    {
        if (codepoint >= atlas.characters.size())
        {
            continue;
        }

        const Character& ch = atlas.characters[codepoint];

        float xpos = pen_x + static_cast<float>(ch.bearing.x);
        float w = static_cast<float>(ch.size.x);
        float h = static_cast<float>(ch.size.y);

        if (w > 0.0f || h > 0.0f)
        {
            min_x = has_geometry ? std::min(min_x, xpos) : xpos;
            max_x = has_geometry ? std::max(max_x, xpos + w) : xpos + w;
            has_geometry = true;
        }

        pen_x += static_cast<float>(ch.advance >> 6);
    }

    if (has_geometry)
    {
        float left_extent = std::min(0.0f, min_x);
        float right_extent = std::max(max_x, pen_x);
        dims.width = right_extent - left_extent;
    }
    else
    {
        dims.width = pen_x;
    }

    float line_height = atlas.line_height > 0.0f ? atlas.line_height : static_cast<float>(config->fontSize);
    if (line_height <= 0.0f)
    {
        line_height = static_cast<float>(config->fontSize);
    }
    dims.height = line_height;

    return dims;
}


















