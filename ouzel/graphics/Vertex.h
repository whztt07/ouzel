// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <string>
#include <vector>
#include "graphics/DataType.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Color.h"

namespace ouzel
{
    namespace graphics
    {
        struct VertexAttribute
        {
            VertexAttribute(const std::string& aName, DataType aDataType, bool aNormalized):
                name(aName), dataType(aDataType), normalized(aNormalized) { }
            std::string name;
            DataType dataType = DataType::NONE;
            bool normalized = false;
        };

        const uint32_t VERTEX_ATTRIBUTE_COUNT = 5;

        class VertexPC
        {
        public:
            static const std::vector<VertexAttribute> ATTRIBUTES;

            Vector3 position;
            Color color;

            VertexPC();
            VertexPC(const Vector3& aPosition, Color aColor);
        };

        class VertexPCT
        {
        public:
            static const std::vector<VertexAttribute> ATTRIBUTES;

            Vector3 position;
            Color color;
            Vector2 texCoord;

            VertexPCT();
            VertexPCT(const Vector3& aPosition, Color aColor, const Vector2& aTexCoord);
        };
    } // namespace graphics
} // namespace ouzel
