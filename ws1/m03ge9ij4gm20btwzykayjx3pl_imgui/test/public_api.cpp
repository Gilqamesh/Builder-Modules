#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4gm20btwzykayjx3pl_imgui/imgui.h>

#include <functional>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        IMGUI_CHECKVERSION();
        test::expect(std::equal_to<>(), std::string(ImGui::GetVersion()), std::string(IMGUI_VERSION));
        test::expect(std::equal_to<>(), IMGUI_VERSION_NUM, 19236);

        const ImVec2 vector2(1.5F, -2.0F);
        test::expect(std::equal_to<>(), vector2.x, 1.5F);
        test::expect(std::equal_to<>(), vector2.y, -2.0F);
        const ImVec4 vector4(1.0F, 2.0F, 3.0F, 4.0F);
        test::expect(std::equal_to<>(), vector4.x, 1.0F);
        test::expect(std::equal_to<>(), vector4.y, 2.0F);
        test::expect(std::equal_to<>(), vector4.z, 3.0F);
        test::expect(std::equal_to<>(), vector4.w, 4.0F);

        ImVector<int> values;
        test::expect(std::identity(), values.empty());
        test::expect(std::equal_to<>(), values.size(), 0);
        values.reserve(8);
        test::expect(std::identity(), 8 <= values.capacity());
        values.push_back(10);
        values.push_back(20);
        values.push_front(5);
        test::expect(std::equal_to<>(), values.size(), 3);
        test::expect(std::equal_to<>(), values.front(), 5);
        test::expect(std::equal_to<>(), values[1], 10);
        test::expect(std::equal_to<>(), values.back(), 20);
        test::expect(std::identity(), values.contains(10));
        test::expect(std::identity(), !values.contains(99));
        test::expect(std::identity(), values.find(20) != values.end());
        test::expect(std::identity(), values.find(99) == values.end());
        values.erase(values.begin() + 1);
        test::expect(std::equal_to<>(), values.size(), 2);
        test::expect(std::equal_to<>(), values[0], 5);
        test::expect(std::equal_to<>(), values[1], 20);
        values.erase_unsorted(values.begin());
        test::expect(std::equal_to<>(), values.size(), 1);
        test::expect(std::equal_to<>(), values[0], 20);
        values.pop_back();
        test::expect(std::identity(), values.empty());
        values.resize(3);
        values[0] = 1;
        values[1] = 2;
        values[2] = 3;
        values.clear();
        test::expect(std::identity(), values.empty());

        ImGuiTextBuffer text_buffer;
        test::expect(std::identity(), text_buffer.empty());
        test::expect(std::equal_to<>(), text_buffer.size(), 0);
        text_buffer.reserve(64);
        text_buffer.append("alpha");
        text_buffer.append(", beta", ", beta" + 6);
        text_buffer.appendf(" %d", 42);
        test::expect(std::equal_to<>(), std::string(text_buffer.c_str()),
            std::string("alpha, beta 42")
        );
        test::expect(std::equal_to<>(), text_buffer.size(), 14);
        test::expect(std::equal_to<>(), text_buffer[0], 'a');
        test::expect(std::equal_to<>(), text_buffer.end() - text_buffer.begin(), 14);
        text_buffer.clear();
        test::expect(std::identity(), text_buffer.empty());
        test::expect(std::equal_to<>(), std::string(text_buffer.c_str()), std::string());

        ImGuiTextFilter filter("-bar,foo");
        test::expect(std::identity(), filter.IsActive());
        test::expect(std::identity(), filter.PassFilter("foo"));
        test::expect(std::identity(), filter.PassFilter("prefix foo suffix"));
        test::expect(std::identity(), !filter.PassFilter("foo and bar"));
        test::expect(std::identity(), !filter.PassFilter("unrelated"));
        filter.Clear();
        test::expect(std::identity(), !filter.IsActive());
        test::expect(std::identity(), filter.PassFilter("anything"));
        std::strcpy(filter.InputBuf, "needle");
        filter.Build();
        test::expect(std::identity(), filter.IsActive());
        test::expect(std::identity(), filter.PassFilter("a needle in text"));
        test::expect(std::identity(), !filter.PassFilter("haystack"));

        ImGuiStorage storage;
        test::expect(std::equal_to<>(), storage.GetInt(1, 7), 7);
        storage.SetInt(1, 42);
        storage.SetBool(2, true);
        storage.SetFloat(3, 1.25F);
        int pointed_value = 9;
        storage.SetVoidPtr(4, &pointed_value);
        test::expect(std::equal_to<>(), storage.GetInt(1), 42);
        test::expect(std::equal_to<>(), storage.GetBool(2), true);
        test::expect(std::equal_to<>(), storage.GetFloat(3), 1.25F);
        test::expect(std::identity(), storage.GetVoidPtr(4) == &pointed_value);
        int* integer_reference = storage.GetIntRef(5, 11);
        test::expect(std::equal_to<>(), *integer_reference, 11);
        *integer_reference = 12;
        test::expect(std::equal_to<>(), storage.GetInt(5), 12);
        float* float_reference = storage.GetFloatRef(6, 2.5F);
        *float_reference = 3.5F;
        test::expect(std::equal_to<>(), storage.GetFloat(6), 3.5F);
        void** pointer_reference = storage.GetVoidPtrRef(7, &pointed_value);
        test::expect(std::identity(), *pointer_reference == &pointed_value);
        storage.BuildSortByKey();
        storage.SetAllInt(-1);
        test::expect(std::equal_to<>(), storage.GetInt(1), -1);
        storage.Clear();
        test::expect(std::equal_to<>(), storage.GetInt(1, 99), 99);

        const ImColor integer_color(255, 128, 0, 255);
        const ImVec4 floating_color = integer_color;
        test::expect(std::identity(), std::abs(floating_color.x - 1.0F) < 0.0001F);
        test::expect(std::identity(), std::abs(floating_color.y - (128.0F / 255.0F)) < 0.0001F);
        test::expect(std::identity(), std::abs(floating_color.z) < 0.0001F);
        test::expect(std::identity(), std::abs(floating_color.w - 1.0F) < 0.0001F);
        const ImU32 packed_color = integer_color;
        test::expect(std::not_equal_to<>(), packed_color, ImU32(0));
        const ImColor hsv_color = ImColor::HSV(0.0F, 1.0F, 1.0F, 1.0F);
        const ImVec4 hsv_vector = hsv_color;
        test::expect(std::identity(), std::abs(hsv_vector.x - 1.0F) < 0.0001F);
        test::expect(std::identity(), std::abs(hsv_vector.y) < 0.0001F);
        test::expect(std::identity(), std::abs(hsv_vector.z) < 0.0001F);

        ImGuiPayload payload;
        payload.Clear();
        test::expect(std::identity(), payload.Data == nullptr);
        test::expect(std::equal_to<>(), payload.DataSize, 0);
        test::expect(std::identity(), !payload.IsDataType("TYPE"));
        test::expect(std::identity(), !payload.IsPreview());
        test::expect(std::identity(), !payload.IsDelivery());

        void* allocation = ImGui::MemAlloc(64);
        test::expect(std::identity(), allocation != nullptr);
        std::memset(allocation, 0x5a, 64);
        ImGui::MemFree(allocation);

        test::expect(std::identity(), ImGui::GetCurrentContext() == nullptr);
        ImGuiContext* context = ImGui::CreateContext();
        test::expect(std::identity(), context != nullptr);
        test::expect(std::identity(), ImGui::GetCurrentContext() == context);
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        test::expect(std::identity(), io.Fonts != nullptr);

        ImGui::StyleColorsDark(&style);
        test::expect(std::equal_to<>(), style.Alpha, 1.0F);
        ImGui::StyleColorsLight(&style);
        test::expect(std::equal_to<>(), style.Alpha, 1.0F);
        ImGui::StyleColorsClassic(&style);
        test::expect(std::equal_to<>(), style.Alpha, 1.0F);
        style.ScaleAllSizes(2.0F);
        test::expect(std::identity(), 0.0F < style.WindowPadding.x);
        test::expect(std::identity(), 0.0F < style.WindowPadding.y);

        ImGui::SetCurrentContext(nullptr);
        test::expect(std::identity(), ImGui::GetCurrentContext() == nullptr);
        ImGui::SetCurrentContext(context);
        test::expect(std::identity(), ImGui::GetCurrentContext() == context);
        ImGui::DestroyContext(context);
        test::expect(std::identity(), ImGui::GetCurrentContext() == nullptr);

        ImFontAtlas* shared_atlas = IM_NEW(ImFontAtlas)();
        ImGuiContext* shared_context = ImGui::CreateContext(shared_atlas);
        test::expect(std::identity(), ImGui::GetIO().Fonts == shared_atlas);
        ImGui::DestroyContext(shared_context);
        test::expect(std::identity(), ImGui::GetCurrentContext() == nullptr);
    });
}
