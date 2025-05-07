#pragma once

// additions for imgui

#include <imgui.h>
#include <vector>
#include <string>
#include <functional>

namespace ImGui
{
    void HelpMarker(const char* desc);

    struct ExampleAppLog
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool                AutoScroll;  // Keep scrolling if already at the bottom.

        ExampleAppLog()
        {
            AutoScroll = true;
            Clear();
        }

        void    Clear()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            if (Buf.size() > 10000)
            {
                Clear();
            }

            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n')
                    LineOffsets.push_back(old_size + 1);
        }

        void    Draw(const char* title, bool* p_open = NULL)
        {
            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (clear)
                    Clear();
                if (copy)
                    ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                const char* buf = Buf.begin();
                const char* buf_end = Buf.end();
                if (Filter.IsActive())
                {
                    // In this example we don't use the clipper when Filter is enabled.
                    // This is because we don't have random access to the result of our filter.
                    // A real application processing logs with ten of thousands of entries may want to store the result of
                    // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                    for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        if (Filter.PassFilter(line_start, line_end))
                            ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                else
                {
                    // The simplest and easy way to display the entire buffer:
                    //   ImGui::TextUnformatted(buf_begin, buf_end);
                    // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                    // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                    // within the visible area.
                    // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                    // on your side is recommended. Using ImGuiListClipper requires
                    // - A) random access into your data
                    // - B) items all being the  same height,
                    // both of which we can handle since we have an array pointing to the beginning of each line of text.
                    // When using the filter (in the block of code above) we don't have random access into the data to display
                    // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                    // it possible (and would be recommended if you want to search through tens of thousands of entries).
                    ImGuiListClipper clipper;
                    clipper.Begin(LineOffsets.Size);
                    while (clipper.Step())
                    {
                        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                        {
                            const char* line_start = buf + LineOffsets[line_no];
                            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                            ImGui::TextUnformatted(line_start, line_end);
                        }
                    }
                    clipper.End();
                }
                ImGui::PopStyleVar();

                // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
                // Using a scrollbar or mouse-wheel will take away from the bottom edge.
                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }
    };
    template<class T>
    concept dataLike = requires(T t)
    {
        t.size();
        t.data();
        t.begin();
        t.end();
    };

    template<dataLike T>
    inline bool ComboWithFilter(const char* label, int* current_item, const T& items, int popup_max_height_in_items = -1);
}

#include "fuzzy_match.h"
#include <imgui_internal.h>
#include <algorithm>

#define ICON_FA_SEARCH "\xef\x80\x82"	// U+f002
namespace ImGui
{

    static bool sortbysec_desc(const std::pair<int, int>& a, const std::pair<int, int>& b)
    {
        return (b.second < a.second);
    }

    static int index_of_key(
        std::vector<std::pair<int, int> > pair_list,
        int key)
    {
        for (int i = 0; i < pair_list.size(); ++i)
        {
            auto& p = pair_list[i];
            if (p.first == key)
            {
                return i;
            }
        }
        return -1;
    }

    // Copied from imgui_widgets.cpp
    static float CalcMaxPopupHeightFromItemCount(int items_count)
    {
        ImGuiContext& g = *GImGui;
        if (items_count <= 0)
            return FLT_MAX;
        return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
    }

    template<dataLike T>
    bool ComboWithFilter(const char* label, int* current_item, const T& items, int popup_max_height_in_items /*= -1 */)
    {
        using namespace fts;

        ImGuiContext& g = *GImGui;

        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiStyle& style = g.Style;

        int items_count = static_cast<int>(items.size());

        // Use imgui Items_ getters to support more input formats.
        const char* preview_value = NULL;
        if (*current_item >= 0 && *current_item < items_count)
            preview_value = items[*current_item].data();

        static int focus_idx = -1;
        static char pattern_buffer[256] = { 0 };

        bool value_changed = false;

        const ImGuiID id = window->GetID(label);
        const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id); // copied from BeginCombo
        const bool is_already_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
        const bool is_filtering = is_already_open && pattern_buffer[0] != '\0';

        int show_count = items_count;

        std::vector<std::pair<int, int> > itemScoreVector;
        if (is_filtering)
        {
            // Filter before opening to ensure we show the correct size window.
            // We won't get in here unless the popup is open.
            for (int i = 0; i < items_count; i++)
            {
                int score = 0;
                bool matched = fuzzy_match(pattern_buffer, items[i].data(), score);
                if (matched)
                    itemScoreVector.push_back(std::make_pair(i, score));
            }
            std::sort(itemScoreVector.begin(), itemScoreVector.end(), sortbysec_desc);
            int current_score_idx = index_of_key(itemScoreVector, focus_idx);
            if (current_score_idx < 0 && !itemScoreVector.empty())
            {
                focus_idx = itemScoreVector[0].first;
            }
            show_count = static_cast<int>(itemScoreVector.size());
        }

        // Define the height to ensure our size calculation is valid.
        if (popup_max_height_in_items == -1) {
            popup_max_height_in_items = 5;
        }
        popup_max_height_in_items = ImMin(popup_max_height_in_items, show_count);


        if (!(g.NextWindowData.ChildFlags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        {
            int items = popup_max_height_in_items + 2; // extra for search bar
            SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(items)));
        }

        if (!BeginCombo(label, preview_value, ImGuiComboFlags_None))
            return false;


        if (!is_already_open)
        {
            focus_idx = *current_item;
            memset(pattern_buffer, 0, IM_ARRAYSIZE(pattern_buffer));
        }

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(240, 240, 240, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255));
        ImGui::PushItemWidth(-FLT_MIN);
        // Filter input
        if (!is_already_open)
            ImGui::SetKeyboardFocusHere();
        InputText("##ComboWithFilter_inputText", pattern_buffer, 256, ImGuiInputTextFlags_AutoSelectAll);

        const ImVec2 label_size = CalcTextSize(ICON_FA_SEARCH, NULL, true);
        const ImVec2 search_icon_pos(
            ImGui::GetItemRectMax().x - label_size.x - style.ItemInnerSpacing.x * 2,
            window->DC.CursorPos.y + style.FramePadding.y + g.FontSize * 0.3f);
        RenderText(search_icon_pos, ICON_FA_SEARCH);

        ImGui::PopStyleColor(2);

        int move_delta = 0;
        if (IsKeyPressed(ImGuiKey_UpArrow))
        {
            --move_delta;
        }
        else if (IsKeyPressed(ImGuiKey_DownArrow))
        {
            ++move_delta;
        }
        else if (IsKeyPressed(ImGuiKey_PageUp))
        {
            move_delta -= popup_max_height_in_items;
        }
        else if (IsKeyPressed(ImGuiKey_PageDown))
        {
            move_delta += popup_max_height_in_items;
        }

        if (move_delta != 0)
        {
            if (is_filtering)
            {
                int current_score_idx = index_of_key(itemScoreVector, focus_idx);
                if (current_score_idx >= 0)
                {
                    const int count = static_cast<int>(itemScoreVector.size());
                    current_score_idx = ImClamp(current_score_idx + move_delta, 0, count - 1);
                    focus_idx = itemScoreVector[current_score_idx].first;
                }
            }
            else
            {
                focus_idx = ImClamp(focus_idx + move_delta, 0, items_count - 1);
            }
        }

        // Copied from ListBoxHeader
        // If popup_max_height_in_items == -1, default height is maximum 7.
        float height_in_items_f = (popup_max_height_in_items < 0 ? ImMin(items_count, 7) : popup_max_height_in_items) + 0.25f;
        ImVec2 size;
        size.x = 0.0f;
        size.y = GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f;

        if (ImGui::BeginListBox("##ComboWithFilter_itemList", size))
        {
            for (int i = 0; i < show_count; i++)
            {
                int idx = is_filtering ? itemScoreVector[i].first : i;
                PushID((void*)(intptr_t)idx);
                const bool item_selected = (idx == focus_idx);
                const char* item_text = items[idx].data();
                if (Selectable(item_text, item_selected))
                {
                    value_changed = true;
                    *current_item = idx;
                    CloseCurrentPopup();
                }

                if (item_selected)
                {
                    SetItemDefaultFocus();
                    // SetItemDefaultFocus doesn't work so also check IsWindowAppearing.
                    if (move_delta != 0 || IsWindowAppearing())
                    {
                        SetScrollHereY();
                    }
                }
                PopID();
            }
            ImGui::EndListBox();

            if (IsKeyPressed(ImGuiKey_Enter))
            {
                value_changed = true;
                *current_item = focus_idx;
                CloseCurrentPopup();
            }
            else if (IsKeyPressed(ImGuiKey_Escape))
            {
                value_changed = false;
                CloseCurrentPopup();
            }
        }
        ImGui::PopItemWidth();
        ImGui::EndCombo();


        if (value_changed)
            MarkItemEdited(g.LastItemData.ID);

        return value_changed;
    }
}

namespace ImGui
{
    ImVec2 WidgetWithResizeHandle(const char* id, std::function<void()> widget_function, float handle_size_em = 1.0f);
    void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
    void EndGroupPanel();
}

struct TabRender
{
    TabRender() = default;
    TabRender(const char* name, const std::function<void()>& func)
        : m_name{ name }, m_func{ func }
    {
    }
    TabRender(const char* name)
        : m_name{ name }, m_func{ nullptr }
    {
    }

    [[nodiscard]] bool funcExist() const
    {
        return m_func != nullptr;
    }

    const char* m_name;
    std::function<void()> m_func;
};
