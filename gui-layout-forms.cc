/*
   This implements the forms layout.
   Widgets are drawn in a design window and are anchored to the window edges. 
   It doesn't handle all layouts but is easy to understand and easy
   to tweak in a wysiwyg layout editor.
*/
#include <cassert>
#include <optional>

struct Size {
    int cx;
    int cy;
};

struct Rect {
    int left;
    int top;
    int right;
    int bottom;

    Size Size() const;
};

enum class Anchor
{
    Top = 0x1 << 1,
    Bottom = 0x1 << 2,
    Left = 0x1 << 3,
    Right = 0x1 << 4,

    TopLeft = Top | Left,
    TopLeftRight = Top | Left | Right,
    TopRight = Top | Right,
    BottomLeft = Bottom | Left,
    BottomLeftRight = Bottom | Left | Right,
    BottomRight = Bottom | Right,
    TopBottomRight = Top | Bottom | Right,
    TopBottomLeft = Top | Bottom | Left,
    All = Top | Bottom | Left | Right
};
bool operator&(Anchor, Anchor);

struct SizeInfo
{
    // horizontal data is either {left+width} {right+width} {left+right}
    // vertical data is either {top+height} {bottom+height} {top+bottom}

    // specify desired offset from the respective edge
    std::optional<int> left;
    std::optional<int> top;
    std::optional<int> right;
    std::optional<int> bottom;

    // if set, specify the fixed widget size
    std::optional<int> width;
    std::optional<int> height;
};

SizeInfo size_info_from_design_information(const Rect& widget_rect, Anchor anchor, Rect design_size)
{
    std::optional<int> left; std::optional<int> right;
    std::optional<int> top; std::optional<int> bottom;

    int vertical = 0;
    int horizontal = 0;

    if (anchor & Anchor::Left)
    {
        left = widget_rect.left - design_size.left;
        horizontal += 1;
    }
    if (anchor & Anchor::Right)
    {
        right = design_size.right - widget_rect.right;
        horizontal += 1;
    }
    if (anchor & Anchor::Top)
    {
        top = widget_rect.top - design_size.top;
        vertical += 1;
    }
    if (anchor & Anchor::Bottom)
    {
        bottom = design_size.bottom - widget_rect.bottom;
        vertical += 1;
    }

    assert(horizontal > 0 && "need to set at least 1 horizontal");
    assert(vertical > 0 && "need to set at least 1 vertical");

    const auto size = widget_rect.Size();
    const auto dx = horizontal < 2 ? std::optional<int>{size.cx} : std::nullopt;
    const auto dy = vertical < 2 ? std::optional<int>{size.cy} : std::nullopt;
    return {left, top, right, bottom, dx, dy};
}

Rect position_widget(const SizeInfo& info, const Rect& window_size)
{
    std::optional<int> left; std::optional<int> right;
    std::optional<int> top; std::optional<int> bottom;

    if (info.left)
    {
        left = *info.left + window_size.left;
    }
    if (info.right)
    {
        right = window_size.right - *info.right;
    }
    if (info.width)
    {
        if (right)
        {
            left = *right - *info.width;
        }
        else
        {
            assert(left);
            right = *left + *info.width;
        }
    }

    if (info.top)
    {
        top = *info.top + window_size.top;
    }
    if (info.bottom)
    {
        bottom = window_size.bottom - *info.bottom;
    }

    if (info.height)
    {
        if (bottom)
        {
            top = *bottom - *info.height;
        }
        else
        {
            assert(top);
            bottom = *top + *info.height;
        }
    }

    assert(left); assert(right);
    assert(top); assert(bottom);

    return {*left, *top, *right, *bottom};
}

