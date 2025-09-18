#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "text.h"
#include "rect.h"
#include "clay.h"
#include "clay_gl.h"

int window_width;
int window_height;

ClayRenderCtx render_ctx;

// Input variables
glm::vec2 scroll;
glm::vec2 mouse_position;
bool mouse_button_down;
bool mouse_button_pressed_this_frame;

float last_frame_time; // in seconds

typedef struct {
    Clay_String title;
    Clay_String contents;
} Document;

typedef struct {
    Document *documents;
    uint32_t length;
} DocumentArray;

Document documents_raw[5];
uint32_t selected_document_index = 0;

DocumentArray documents = {
    .documents = documents_raw,
    .length = 5,
};

const Clay_Color COLOR_WHITE = { 255, 255, 255, 255 };
const Clay_Color COLOR_TRANSPARENT = { 0, 0, 0, 0 };

Clay_Color primary_color = { 89, 255, 0, 255 };
Clay_Color secondary_color = { 255, 0, 255, 255 };
Clay_Color text_color = { 255, 255, 255, 255 };

struct SliderState {
    float* valuePtr = nullptr;
    float min = 0.0f;
    float max = 1.0f;
    bool dragging = false;
    uint32_t idIndex = 0;
    std::array<char, 64> labelBuffer{};
};

static std::map<const char*, SliderState> g_sliderStates;
static uint32_t g_sliderNextId = 1;

struct TextInputState {
    std::array<char, 128> buffer{};
    uint32_t length = 0;
    uint32_t cachedDocumentIndex = std::numeric_limits<uint32_t>::max();
    bool focused = false;
    bool dirty = false;
};

static TextInputState header_text_input_state;

void slider_component(Clay_String text, float* value, float min, float max);
void slider_track_interaction(Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data);

void text_box_component(uint16_t font_id, uint16_t font_size);
void char_input_callback(GLFWwindow* window, unsigned int codepoint);
void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

Clay_RenderCommandArray ui_update(ClayRenderCtx* ctx);

void HandleClayErrors(Clay_ErrorData errorData);

Clay_Color get_color_shade(Clay_Color base, float shade);

float get_frame_time();

void render_header_button(Clay_String text);

void handle_sidebar_interaction(Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data);

Clay_RenderCommandArray create_layout();



int main() 
{
    documents.documents[0] = { .title = CLAY_STRING("Squirrels"), .contents = CLAY_STRING("The Secret Life of Squirrels: Nature's Clever Acrobats\n""Squirrels are often overlooked creatures, dismissed as mere park inhabitants or backyard nuisances. Yet, beneath their fluffy tails and twitching noses lies an intricate world of cunning, agility, and survival tactics that are nothing short of fascinating. As one of the most common mammals in North America, squirrels have adapted to a wide range of environments from bustling urban centers to tranquil forests and have developed a variety of unique behaviors that continue to intrigue scientists and nature enthusiasts alike.\n""\n""Master Tree Climbers\n""At the heart of a squirrel's skill set is its impressive ability to navigate trees with ease. Whether they're darting from branch to branch or leaping across wide gaps, squirrels possess an innate talent for acrobatics. Their powerful hind legs, which are longer than their front legs, give them remarkable jumping power. With a tail that acts as a counterbalance, squirrels can leap distances of up to ten times the length of their body, making them some of the best aerial acrobats in the animal kingdom.\n""But it's not just their agility that makes them exceptional climbers. Squirrels' sharp, curved claws allow them to grip tree bark with precision, while the soft pads on their feet provide traction on slippery surfaces. Their ability to run at high speeds and scale vertical trunks with ease is a testament to the evolutionary adaptations that have made them so successful in their arboreal habitats.\n""\n""Food Hoarders Extraordinaire\n""Squirrels are often seen frantically gathering nuts, seeds, and even fungi in preparation for winter. While this behavior may seem like instinctual hoarding, it is actually a survival strategy that has been honed over millions of years. Known as \"scatter hoarding,\" squirrels store their food in a variety of hidden locations, often burying it deep in the soil or stashing it in hollowed-out tree trunks.\n""Interestingly, squirrels have an incredible memory for the locations of their caches. Research has shown that they can remember thousands of hiding spots, often returning to them months later when food is scarce. However, they don't always recover every stash some forgotten caches eventually sprout into new trees, contributing to forest regeneration. This unintentional role as forest gardeners highlights the ecological importance of squirrels in their ecosystems.\n""\n""The Great Squirrel Debate: Urban vs. Wild\n""While squirrels are most commonly associated with rural or wooded areas, their adaptability has allowed them to thrive in urban environments as well. In cities, squirrels have become adept at finding food sources in places like parks, streets, and even garbage cans. However, their urban counterparts face unique challenges, including traffic, predators, and the lack of natural shelters. Despite these obstacles, squirrels in urban areas are often observed using human infrastructure such as buildings, bridges, and power lines as highways for their acrobatic escapades.\n""There is, however, a growing concern regarding the impact of urban life on squirrel populations. Pollution, deforestation, and the loss of natural habitats are making it more difficult for squirrels to find adequate food and shelter. As a result, conservationists are focusing on creating squirrel-friendly spaces within cities, with the goal of ensuring these resourceful creatures continue to thrive in both rural and urban landscapes.\n""\n""A Symbol of Resilience\n""In many cultures, squirrels are symbols of resourcefulness, adaptability, and preparation. Their ability to thrive in a variety of environments while navigating challenges with agility and grace serves as a reminder of the resilience inherent in nature. Whether you encounter them in a quiet forest, a city park, or your own backyard, squirrels are creatures that never fail to amaze with their endless energy and ingenuity.\n""In the end, squirrels may be small, but they are mighty in their ability to survive and thrive in a world that is constantly changing. So next time you spot one hopping across a branch or darting across your lawn, take a moment to appreciate the remarkable acrobat at work a true marvel of the natural world.\n") };
    documents.documents[1] = { .title = CLAY_STRING("Lorem Ipsum"), .contents = CLAY_STRING("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.") };
    documents.documents[2] = { .title = CLAY_STRING("Vacuum Instructions"), .contents = CLAY_STRING("Chapter 3: Getting Started - Unpacking and Setup\n""\n""Congratulations on your new SuperClean Pro 5000 vacuum cleaner! In this section, we will guide you through the simple steps to get your vacuum up and running. Before you begin, please ensure that you have all the components listed in the \"Package Contents\" section on page 2.\n""\n""1. Unboxing Your Vacuum\n""Carefully remove the vacuum cleaner from the box. Avoid using sharp objects that could damage the product. Once removed, place the unit on a flat, stable surface to proceed with the setup. Inside the box, you should find:\n""\n""    The main vacuum unit\n""    A telescoping extension wand\n""    A set of specialized cleaning tools (crevice tool, upholstery brush, etc.)\n""    A reusable dust bag (if applicable)\n""    A power cord with a 3-prong plug\n""    A set of quick-start instructions\n""\n""2. Assembling Your Vacuum\n""Begin by attaching the extension wand to the main body of the vacuum cleaner. Line up the connectors and twist the wand into place until you hear a click. Next, select the desired cleaning tool and firmly attach it to the wand's end, ensuring it is securely locked in.\n""\n""For models that require a dust bag, slide the bag into the compartment at the back of the vacuum, making sure it is properly aligned with the internal mechanism. If your vacuum uses a bagless system, ensure the dust container is correctly seated and locked in place before use.\n""\n""3. Powering On\n""To start the vacuum, plug the power cord into a grounded electrical outlet. Once plugged in, locate the power switch, usually positioned on the side of the handle or body of the unit, depending on your model. Press the switch to the \"On\" position, and you should hear the motor begin to hum. If the vacuum does not power on, check that the power cord is securely plugged in, and ensure there are no blockages in the power switch.\n""\n""Note: Before first use, ensure that the vacuum filter (if your model has one) is properly installed. If unsure, refer to \"Section 5: Maintenance\" for filter installation instructions.") };
    documents.documents[3] = { .title = CLAY_STRING("Article 4"), .contents = CLAY_STRING("Article 4") };
    documents.documents[4] = { .title = CLAY_STRING("Article 5"), .contents = CLAY_STRING("Article 5") };

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_width = 1200;
    window_height = 800;

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Clay UI", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }  

    glViewport(0, 0, window_width, window_height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCharCallback(window, char_input_callback);
    glfwSetKeyCallback(window, key_input_callback);

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));

    // Note: screenWidth and screenHeight will need to come from your environment, Clay doesn't handle window related tasks
    Clay_Initialize(arena, { (float)window_width, (float)window_height }, { HandleClayErrors });

    std::vector<std::string> image_filepaths = { 
        "images/pikachu.png",
        "images/all_might.jpg",
    };
    std::vector<std::string> font_filepaths = {
        "fonts/arial.ttf",
    };
    clay_init_render_ctx(&render_ctx, image_filepaths, font_filepaths);


    while(!glfwWindowShouldClose(window))
    {
        process_input(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);   
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Clay_RenderCommandArray commands = create_layout();

        clay_render(commands, &render_ctx, window_width, window_height);

        glfwSwapBuffers(window);
        glfwPollEvents();  
        
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
}  

void process_input(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    double xpos = 0.0;
    double ypos = 0.0;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouse_position = glm::vec2{ static_cast<float>(xpos), static_cast<float>(ypos) };

    bool is_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    static bool previous_mouse_button_state = false;
    mouse_button_pressed_this_frame = !previous_mouse_button_state && is_down;
    mouse_button_down = is_down;
    previous_mouse_button_state = is_down;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) 
{
    scroll.x += xoffset;
    scroll.y += yoffset;
}

void HandleClayErrors(Clay_ErrorData errorData)
{
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    switch(errorData.errorType) {
        // etc
    }
}

float get_frame_time()
{
    float current_time = static_cast<float>(glfwGetTime());

    if (last_frame_time == 0.0f) {
        last_frame_time = current_time;
        return 0.0f;
    }

    float delta = current_time - last_frame_time;
    last_frame_time = current_time;
    return delta;
}

Clay_Color get_color_shade(Clay_Color base, float shade)
{
    return { base.r * shade, base.g * shade, base.b * shade, base.a };
}

void render_header_button(Clay_String text)
{
    CLAY_AUTO_ID({
        .layout = {
            .padding = { 16, 16, 8, 8},

        },
        .backgroundColor = get_color_shade(primary_color, 140.0f / 255.0f),
        .cornerRadius = { 5, 5, 5, 5 },
    }) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .userData = (void*)&render_ctx,
            .textColor = COLOR_WHITE,
            .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
            .fontSize = 16,
        }));
    }
}

void render_dropdown_menu_item(Clay_String text)
{
    CLAY_AUTO_ID({
        .layout = {
            .padding = { 16, 16, 8, 8},

        },
    }) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .userData = (void*)&render_ctx,
            .textColor = COLOR_WHITE,
            .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
            .fontSize = 16,
        }));
    }
}

void handle_sidebar_interaction(Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data)
{
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        if (user_data >= 0 && user_data < documents.length)
        {
            selected_document_index = user_data;
        }
    }
}

void slider_track_interaction(Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data)
{
    SliderState* state = reinterpret_cast<SliderState*>(user_data);
    if (!state || !state->valuePtr)
    {
        return;
    }

    bool pointerDown = pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME
        || pointer_data.state == CLAY_POINTER_DATA_PRESSED;

    if (state->dragging || pointerDown)
    {
        Clay_ElementData elementData = Clay_GetElementData(element_id);
        if (elementData.found && elementData.boundingBox.width > 0.0f)
        {
            float relative = (pointer_data.position.x - elementData.boundingBox.x) / elementData.boundingBox.width;
            relative = std::clamp(relative, 0.0f, 1.0f);
            float range = state->max - state->min;
            float newValue = range != 0.0f ? state->min + relative * range : state->min;
            *state->valuePtr = std::clamp(newValue, state->min, state->max);
        }
    }

    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        state->dragging = true;
    }
    else if (pointer_data.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME
        || pointer_data.state == CLAY_POINTER_DATA_RELEASED)
    {
        state->dragging = false;
    }
}

// Slider that allows the user to adjust the value of a variable
void slider_component(Clay_String text, float* value, float min, float max)
{
    if (!value)
    {
        return;
    }

    float rangeMin = std::min(min, max);
    float rangeMax = std::max(min, max);

    SliderState& state = g_sliderStates[text.chars];
    if (state.idIndex == 0)
    {
        state.idIndex = g_sliderNextId++;
    }

    state.valuePtr = value;
    state.min = rangeMin;
    state.max = rangeMax;

    if (!mouse_button_down)
    {
        state.dragging = false;
    }

    *value = std::clamp(*value, rangeMin, rangeMax);

    float range = rangeMax - rangeMin;
    float normalized = range > 0.0f ? (*value - rangeMin) / range : 0.0f;
    normalized = std::clamp(normalized, 0.0f, 1.0f);

    int written = std::snprintf(
        state.labelBuffer.data(),
        state.labelBuffer.size(),
        "%.*s: %.0f",
        static_cast<int>(text.length),
        text.chars,
        *value);

    if (written < 0)
    {
        written = 0;
        state.labelBuffer[0] = '\0';
    }
    else if (written >= static_cast<int>(state.labelBuffer.size()))
    {
        written = static_cast<int>(state.labelBuffer.size()) - 1;
    }

    Clay_String labelString = {
        .isStaticallyAllocated = false,
        .length = static_cast<int32_t>(written),
        .chars = state.labelBuffer.data(),
    };

    Clay_ElementId trackId = Clay_GetElementIdWithIndex(CLAY_STRING("SliderTrack"), state.idIndex);
    bool pointerOver = Clay_PointerOver(trackId);
    float trackShade = (pointerOver || state.dragging) ? 75.0f / 255.0f : 60.0f / 255.0f;

    CLAY_AUTO_ID({
        .layout = {
            .sizing = { .width = CLAY_SIZING_GROW() },
            .childGap = 6,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    }) {
        CLAY_TEXT(labelString, CLAY_TEXT_CONFIG({
            .userData = (void*)&render_ctx,
            .textColor = COLOR_WHITE,
            .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
            .fontSize = 14,
        }));

        CLAY(CLAY_IDI("SliderTrack", state.idIndex), {
            .layout = {
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIXED(18) },
                .padding = { 4, 4, 4, 4 },
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            },
            .backgroundColor = get_color_shade(primary_color, trackShade),
            .cornerRadius = CLAY_CORNER_RADIUS(6),
        }) {
            Clay_OnHover(slider_track_interaction, (intptr_t)(&state));

            CLAY_AUTO_ID({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(normalized),
                        .height = CLAY_SIZING_GROW(),
                    },
                },
                .backgroundColor = get_color_shade(primary_color, 140.0f / 255.0f),
                .cornerRadius = CLAY_CORNER_RADIUS(4),
            }) {}
        }
    }
}

// Text box component, create additional parameters if necessary
void text_box_component(uint16_t font_id, uint16_t font_size)
{
    TextInputState& state = header_text_input_state;

    if (!state.focused && !state.dirty && selected_document_index < documents.length)
    {
        if (state.cachedDocumentIndex != selected_document_index)
        {
            Document selected = documents.documents[selected_document_index];
            size_t copy_len = std::min(static_cast<size_t>(selected.title.length), state.buffer.size() - 1);
            if (copy_len > 0)
            {
                std::memcpy(state.buffer.data(), selected.title.chars, copy_len);
                state.length = static_cast<uint32_t>(copy_len);
                state.buffer[copy_len] = '\0';
            }
            else
            {
                state.length = 0;
                state.buffer[0] = '\0';
            }
            state.dirty = false;
            state.cachedDocumentIndex = selected_document_index;
        }
    }

    const char placeholder[] = "Search documents";
    bool has_value = state.length > 0;
    const char* display_chars = has_value ? state.buffer.data() : placeholder;
    int32_t display_length = has_value
        ? static_cast<int32_t>(state.length)
        : static_cast<int32_t>(std::strlen(placeholder));

    Clay_String display_string = {
        .isStaticallyAllocated = has_value ? false : true,
        .length = display_length,
        .chars = display_chars,
    };

    Clay_ElementId textbox_id = Clay_GetElementId(CLAY_STRING("HeaderTextBox"));
    bool pointer_over = Clay_PointerOver(textbox_id);

    if (mouse_button_pressed_this_frame)
    {
        if (pointer_over)
        {
            state.focused = true;
            state.cachedDocumentIndex = std::numeric_limits<uint32_t>::max();
        }
        else
        {
            state.focused = false;
        }
    }

    Clay_Color base_background = get_color_shade(primary_color, 70.0f / 255.0f);
    Clay_Color hover_background = get_color_shade(primary_color, 100.0f / 255.0f);
    Clay_Color active_background = get_color_shade(primary_color, 140.0f / 255.0f);
    Clay_Color border_color = state.focused
        ? get_color_shade(primary_color, 180.0f / 255.0f)
        : get_color_shade(primary_color, 140.0f / 255.0f);
    Clay_Color background_color = base_background;
    if (state.focused)
    {
        background_color = active_background;
    }
    else if (pointer_over)
    {
        background_color = hover_background;
    }

    Clay_Color text_display_color = has_value ? text_color : get_color_shade(text_color, 140.0f / 255.0f);

    CLAY(CLAY_ID("HeaderTextBox"), {
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(260), .height = CLAY_SIZING_FIXED(36) },
            .padding = { 12, 12, 8, 8 },
            .childGap = 10,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = background_color,
        .cornerRadius = CLAY_CORNER_RADIUS(8),
        .border = { .color = border_color, .width = { 1, 1, 1, 1, 0 } },
    }) {
        CLAY_AUTO_ID({
            .layout = {
                .sizing = { .width = CLAY_SIZING_FIXED(6), .height = CLAY_SIZING_GROW() },
            },
            .backgroundColor = get_color_shade(primary_color, 160.0f / 255.0f),
            .cornerRadius = CLAY_CORNER_RADIUS(3),
        }) {}

        CLAY_TEXT(display_string, CLAY_TEXT_CONFIG({
            .userData = (void*)&render_ctx,
            .textColor = text_display_color,
            .fontId = font_id,
            .fontSize = font_size,
        }));
    }
}


void char_input_callback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    TextInputState& state = header_text_input_state;
    if (!state.focused)
    {
        return;
    }

    if (codepoint < 32 || codepoint > 126)
    {
        return;
    }

    if (state.length >= state.buffer.size() - 1)
    {
        return;
    }

    state.buffer[state.length++] = static_cast<char>(codepoint);
    state.buffer[state.length] = '\0';
    state.cachedDocumentIndex = std::numeric_limits<uint32_t>::max();
    state.dirty = true;
}

void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }

    TextInputState& state = header_text_input_state;

    if (key == GLFW_KEY_BACKSPACE)
    {
        if (state.focused && state.length > 0)
        {
            state.length--;
            state.buffer[state.length] = '\0';
            state.cachedDocumentIndex = std::numeric_limits<uint32_t>::max();
        }
        if (state.focused)
        {
            state.dirty = state.length > 0;
        }
        return;
    }

    if (!state.focused)
    {
        return;
    }

    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER)
    {
        state.focused = false;
    }
}

void sidebar_documents_component()
{
    Clay_LayoutConfig sidebar_button_layout = {
        .sizing = { .width = CLAY_SIZING_GROW() }, 
        .padding = { 16, 16, 16, 16 }
    };
    Clay_CornerRadius sidebar_button_cr = { 8, 8, 8, 8 };

    for (int i = 0; i < documents.length; i++) 
    {
        Document document = documents.documents[i];
        if (i == selected_document_index)
        {
            CLAY_AUTO_ID({
                .layout = sidebar_button_layout,
                .backgroundColor = get_color_shade(primary_color, 120.0f / 255.0f),
                .cornerRadius = sidebar_button_cr,
                .border = {
                    .color = primary_color,
                    .width = { 2, 2, 2, 2, 0 },
                }
            }) {
                CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                    .textColor = COLOR_WHITE,
                    .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
                    .fontSize = 16,
                }));
            }
        } else
        {
            Clay_Color hovered = { 120, 120, 120, 120 };
            CLAY_AUTO_ID({
                .layout = sidebar_button_layout,
                .backgroundColor = Clay_Hovered() ? hovered : COLOR_TRANSPARENT,
                .cornerRadius = sidebar_button_cr,
            }) {
                Clay_OnHover(handle_sidebar_interaction, i);
                CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                    .textColor = COLOR_WHITE,
                    .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
                    .fontSize = 16,
                }));
            }
        }
        
    }
}

Clay_RenderCommandArray create_layout()
{
    Clay_Sizing layout_expand = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() };

        Clay_SetLayoutDimensions({ (float)window_width, (float)window_height });
        Clay_SetPointerState( { mouse_position.x, mouse_position.y }, mouse_button_down);
        Clay_UpdateScrollContainers(
            true,
            { scroll.x, scroll.y },
            get_frame_time()
        );
        scroll = { 0.0f, 0.0f };
        
        Clay_BeginLayout(); 

        CLAY(CLAY_ID("OuterContainer"), {
            .layout = {
                .sizing = layout_expand,
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = { 35, 35, 35, 255 },
        }) {
            CLAY(CLAY_ID("HeaderBar"), {
                .layout = {
                    .sizing = { 
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_FIXED(60),
                    },
                    .padding = { 16, 16, 16, 16 },
                    .childGap = 16,
                    .childAlignment = {
                        .y = CLAY_ALIGN_Y_CENTER,
                    }
                },
                .backgroundColor = get_color_shade(primary_color, 90.0f / 255.0f),
                .cornerRadius = { 8, 8, 8, 8 },
            }) {
                CLAY(CLAY_ID("FileButton"), {
                    .layout = {
                        .padding = { 16, 16, 8, 8},

                    },
                    .backgroundColor = get_color_shade(primary_color, 140.0f / 255.0f),
                    .cornerRadius = { 5, 5, 5, 5 },
                }) {
                    CLAY_TEXT(CLAY_STRING("File"), CLAY_TEXT_CONFIG({
                        .userData = (void*)&render_ctx,
                        .textColor = COLOR_WHITE,
                        .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
                        .fontSize = 16,
                    }));

                    bool file_menu_visible = 
                        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileButton")))
                        ||
                        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileMenu")));

                    if (file_menu_visible) 
                    {
                        CLAY(CLAY_ID("FileMenu"), {
                            .layout = {
                                .padding = {0, 0, 8, 8 }
                            },
                            .floating = {
                                .attachPoints = {
                                    .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                                },
                                .attachTo = CLAY_ATTACH_TO_PARENT,
                            },
                        }) {
                            CLAY_AUTO_ID({
                                .layout = {
                                    .sizing = {
                                            .width = CLAY_SIZING_FIXED(200)
                                    },
                                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                },
                                .backgroundColor = get_color_shade(primary_color, 40.0f / 255.0f),
                                .cornerRadius = CLAY_CORNER_RADIUS(8)
                            }) {
                                // Render dropdown items here
                                render_dropdown_menu_item(CLAY_STRING("New"));
                                render_dropdown_menu_item(CLAY_STRING("Open"));
                                render_dropdown_menu_item(CLAY_STRING("Close"));
                            }
                        }
                    }
                    
                }
                render_header_button(CLAY_STRING("Edit"));
                render_header_button(CLAY_STRING("Upload"));
                render_header_button(CLAY_STRING("Media"));
                render_header_button(CLAY_STRING("AV"));

                CLAY_AUTO_ID({
                    .layout = {
                        .sizing = { .width = CLAY_SIZING_GROW() }
                    }
                }) {}

                text_box_component(get_font_id(&render_ctx, "fonts/arial.ttf"), 24);

                
            }
            CLAY(CLAY_ID("LowerContent"), {
                .layout = {
                    .sizing = layout_expand,
                    .childGap = 16,
                },
            }) {
                CLAY(CLAY_ID("Sidebar"), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_FIXED(250),
                            .height = CLAY_SIZING_GROW(),
                        },
                        .padding = { 16, 16, 16, 16 },
                        .childGap = 8,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = get_color_shade(primary_color, 90.0f / 255.0f),
                    .cornerRadius = { 8, 8, 8, 8 },
                }) {
                    CLAY(CLAY_ID("PikachuImage"), {
                        .layout = { .sizing = layout_expand },
                        .backgroundColor = { 255, 255, 255, 255 },
                        .aspectRatio = { .aspectRatio = 1.0f },
                        .image = { .imageData = (void*)"images/pikachu.png" },
                    }) {}

                    sidebar_documents_component();

                    CLAY_AUTO_ID({
                        .layout = { .sizing = layout_expand, },
                    }) {}

                    slider_component(CLAY_STRING("Primary R"), &primary_color.r, 0.0f, 255.0f);
                    slider_component(CLAY_STRING("Primary G"), &primary_color.g, 0.0f, 255.0f);
                    slider_component(CLAY_STRING("Primary B"), &primary_color.b, 0.0f, 255.0f);
                }
                CLAY(CLAY_ID("MainContent"), {
                    .layout = {
                        .sizing = layout_expand,
                        .padding = { 16, 16, 16, 16 },
                        .childGap = 16,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = get_color_shade(primary_color, 90.0f / 255.0f),
                    .cornerRadius = { 8, 8, 8, 8 },
                    .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() },
                }) {
                    Document selectedDocument = documents.documents[selected_document_index];
                    CLAY_TEXT(selectedDocument.title, CLAY_TEXT_CONFIG({
                        .textColor = COLOR_WHITE,
                        .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
                        .fontSize = 24,
                    }));
                    CLAY_TEXT(selectedDocument.contents, CLAY_TEXT_CONFIG({
                        .textColor = COLOR_WHITE,
                        .fontId = get_font_id(&render_ctx, "fonts/arial.ttf"),
                        .fontSize = 24,
                    }));
                }
            }
        }

        return Clay_EndLayout();
}






















