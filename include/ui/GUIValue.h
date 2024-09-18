#pragma once

#include "imgui/imgui.h"
#include <functional>

namespace Vel
{
    template <typename T>
    class GUIValue
    {
    public:
        const T& Get() const
        {
            return val;
        }

        void SetValue(T&& value)
        {
            val = std::forward<T>(value);
        }

        void SetDrawFunc(std::function<void(T*)>&& func)
        {
            imguiDrawFunc = std::forward<std::function<void(T*)>>(func);
        }

        void operator()()
        {
            if(imguiDrawFunc)
                imguiDrawFunc(&val);
        }

    private:
        T val;

        std::function<void(T*)> imguiDrawFunc;
    };
}