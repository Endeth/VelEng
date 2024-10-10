#pragma once

namespace Vel
{
    class Resource
    {
    public:
        void Load();
        void Unload();
    };

    class TextureResource : public Resource
    {
    };

    class GLTFResource : public Resource
    {
    };
}
