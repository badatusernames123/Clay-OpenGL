#pragma once

#include <cstdint>
#include <algorithm>

#include <glm/glm.hpp>

struct Rect
{
    float left;
    float right;
    float top;
    float bot;

    glm::vec2 tl()
    {
        return { left, top };
    }

    glm::vec2 bl()
    {
        return { left, bot };
    }

    glm::vec2 br()
    {
        return { right, bot };
    }

    glm::vec2 tr()
    {
        return { right, top };
    }

    Rect intersection(Rect r)
    {
        Rect res;
        res.left = std::max(left, r.left);
        res.right = std::min(right, r.right);
        res.top = std::max(top, r.top);
        res.bot = std::min(bot, r.bot);
        return res;
    }
};

struct IRect
{
    int left;
    int right;
    int top;
    int bot;

    glm::ivec2 tl()
    {
        return { left, top };
    }

    glm::ivec2 bl()
    {
        return { left, bot };
    }

    glm::ivec2 br()
    {
        return { right, bot };
    }

    glm::ivec2 tr()
    {
        return { right, top };
    }
};