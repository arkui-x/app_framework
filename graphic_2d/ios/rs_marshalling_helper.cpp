/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "transaction/rs_marshalling_helper.h"

#include <memory>
#include <message_parcel.h>
#include <unistd.h>

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkVertices.h"
#ifdef NEW_SKIA
#include "include/core/SkSamplingOptions.h"
#include "src/core/SkVerticesPriv.h"
#endif
#endif
#include "securec.h"
#ifndef USE_ROSEN_DRAWING
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/image/SkImage_Base.h"
#else
#include "recording/recording_shader_effect.h"
#include "recording/recording_path.h"
#endif

#include "animation/rs_render_curve_animation.h"
#include "animation/rs_render_interpolating_spring_animation.h"
#include "animation/rs_render_keyframe_animation.h"
#include "animation/rs_render_path_animation.h"
#include "animation/rs_render_spring_animation.h"
#include "animation/rs_render_transition.h"
#include "common/rs_color.h"
#include "common/rs_common_def.h"
#include "common/rs_matrix3.h"
#include "common/rs_vector4.h"
#include "modifier/rs_render_modifier.h"
#ifndef USE_ROSEN_DRAWING
#include "pipeline/rs_draw_cmd.h"
#include "pipeline/rs_draw_cmd_list.h"
#endif
#include "pixel_map.h"
#include "platform/common/rs_log.h"
#include "render/rs_blur_filter.h"
#include "render/rs_filter.h"
#include "render/rs_gradient_blur_para.h"
#include "render/rs_image.h"
#include "render/rs_image_base.h"
#include "render/rs_light_up_effect_filter.h"
#include "render/rs_material_filter.h"
#include "render/rs_path.h"
#include "render/rs_shader.h"
#include "transaction/rs_ashmem_helper.h"

namespace OHOS {
namespace Rosen {

#define MARSHALLING_AND_UNMARSHALLING(TYPE, TYPENAME)                      \
    bool RSMarshallingHelper::Marshalling(Parcel& parcel, const TYPE& val) \
    {                                                                      \
        return {};                                                         \
    }                                                                      \
    bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, TYPE& val)     \
    {                                                                      \
        return {};                                                         \
    }

// basic types
MARSHALLING_AND_UNMARSHALLING(bool, Bool)
MARSHALLING_AND_UNMARSHALLING(int8_t, Int8)
MARSHALLING_AND_UNMARSHALLING(uint8_t, Uint8)
MARSHALLING_AND_UNMARSHALLING(int16_t, Int16)
MARSHALLING_AND_UNMARSHALLING(uint16_t, Uint16)
MARSHALLING_AND_UNMARSHALLING(int32_t, Int32)
MARSHALLING_AND_UNMARSHALLING(uint32_t, Uint32)
MARSHALLING_AND_UNMARSHALLING(int64_t, Int64)
MARSHALLING_AND_UNMARSHALLING(uint64_t, Uint64)
MARSHALLING_AND_UNMARSHALLING(float, Float)
MARSHALLING_AND_UNMARSHALLING(double, Double)

#undef MARSHALLING_AND_UNMARSHALLING

namespace {
template<typename T, typename P>
static inline sk_sp<T> sk_reinterpret_cast(sk_sp<P> ptr)
{
    return sk_sp<T>(static_cast<T*>(SkSafeRef(ptr.get())));
}
} // namespace

#ifndef USE_ROSEN_DRAWING
// SkData
bool RSMarshallingHelper::Marshalling(Parcel& parcel, sk_sp<SkData> val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkData>& val)
{
    return {};
}
bool RSMarshallingHelper::SkipSkData(Parcel& parcel)
{
    return {};
}

bool RSMarshallingHelper::UnmarshallingWithCopy(Parcel& parcel, sk_sp<SkData>& val)
{
    return {};
}
#else
// Drawing::Data
bool RSMarshallingHelper::Marshalling(Parcel& parcel, std::shared_ptr<Drawing::Data> val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Drawing::Data>& val)
{
    return {};
}

bool RSMarshallingHelper::SkipData(Parcel& parcel)
{
    return {};
}

bool RSMarshallingHelper::UnmarshallingWithCopy(Parcel& parcel, std::shared_ptr<Drawing::Data>& val)
{
    return {};
}
#endif

#ifndef USE_ROSEN_DRAWING
// SkTypeface serial proc
sk_sp<SkData> RSMarshallingHelper::SerializeTypeface(SkTypeface* tf, void* ctx)
{
    return {};
}

// SkTypeface deserial proc
sk_sp<SkTypeface> RSMarshallingHelper::DeserializeTypeface(const void* data, size_t length, void* ctx)
{
    return {};
}

// SkTextBlob
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkTextBlob>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkTextBlob>& val)
{
    return {};
}

// SkPaint
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkPaint& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkPaint& val)
{
    return {};
}
#endif

#ifndef USE_ROSEN_DRAWING
// SkImage
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkImage>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkImage>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkImage>& val, void*& imagepixelAddr)
{
    return {};
}
#else
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<Drawing::Image>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Drawing::Image>& val, void*& imagepixelAddr)
{
    return {};
}
#endif

#ifndef USE_ROSEN_DRAWING
bool RSMarshallingHelper::SkipSkImage(Parcel& parcel)
{
    return {};
}
#else
bool RSMarshallingHelper::SkipImage(Parcel& parcel)
{
    return {};
}
#endif

#ifndef USE_ROSEN_DRAWING
// SkPicture
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkPicture>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkPicture>& val)
{
    return {};
}

// SkVertices
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkVertices>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkVertices>& val)
{
    return {};
}

// SkRect
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkRect& rect)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkRect& rect)
{
    return {};
}

// SkRegion
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkRegion& region)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkRegion& region)
{
    return {};
}

// SkBitmap
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkBitmap& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkBitmap& val)
{
    return {};
}

// SKPath
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkPath& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkPath& val)
{
    return {};
}

// SkFlattenable
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkFlattenable>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkFlattenable>& val)
{
    return {};
}

// SkDrawable
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkDrawable>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkDrawable>& val)
{
    return {};
}

// SkImageFilter
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const sk_sp<SkImageFilter>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, sk_sp<SkImageFilter>& val)
{
    return {};
}
#endif

// RSShader
#ifndef USE_ROSEN_DRAWING
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSShader>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSShader>& val)
{
    return {};
}
#else
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSShader>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSShader>& val)
{
    return {};
}
// Drawing::Matrix
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const Drawing::Matrix& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, Drawing::Matrix& val)
{
    return {};
}
#endif

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSLinearGradientBlurPara>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSLinearGradientBlurPara>& val)
{
    return {};
}

// RSParticle
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const EmitterConfig& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, EmitterConfig& val)
{
    return {};
}
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const ParticleVelocity& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, ParticleVelocity& val)
{
    return {};
}
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const RenderParticleParaType<float>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, RenderParticleParaType<float>& val)
{
    return {};
}
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const RenderParticleColorParaType& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, RenderParticleColorParaType& val)
{
    return {};
}
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<ParticleRenderParams>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<ParticleRenderParams>& val)
{
    return {};
}
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::vector<std::shared_ptr<ParticleRenderParams>>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::vector<std::shared_ptr<ParticleRenderParams>>& val)
{
    return {};
}

// RSPath
#ifndef USE_ROSEN_DRAWING
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSPath>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSPath>& val)
{
    return {};
}
#else
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSPath>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSPath>& val)
{
    return {};
}
#endif

// RSMask
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSMask>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSMask>& val)
{
    return {};
}

// RSFilter
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSFilter>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSFilter>& val)
{
    return {};
}

// RSImageBase
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSImageBase>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSImageBase>& val)
{
    return {};
}

// RSImage
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSImage>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSImage>& val)
{
    return {};
}

// Media::PixelMap
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<Media::PixelMap>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Media::PixelMap>& val)
{
    return {};
}
bool RSMarshallingHelper::SkipPixelMap(Parcel& parcel)
{
    return {};
}

// RectF
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RectT<float>>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RectT<float>>& val)
{
    return {};
}

//RRect
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const RRectT<float>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, RRectT<float>& val)
{
    return {};
}

#ifndef USE_ROSEN_DRAWING
#ifdef NEW_SKIA
// SkPaint
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const SkSamplingOptions& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, SkSamplingOptions& val)
{
    return {};
}
#endif
#endif

#ifdef USE_ROSEN_DRAWING
// Drawing::DrawCmdList
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<Drawing::DrawCmdList>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Drawing::DrawCmdList>& val)
{
    return {};
}
#endif

#define MARSHALLING_AND_UNMARSHALLING(TYPE)                                                 \
    bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TYPE>& val) \
    {                                                                                       \
        return {};                                                                          \
    }                                                                                       \
    bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TYPE>& val)     \
    {                                                                                       \
        return {};                                                                          \
    }
MARSHALLING_AND_UNMARSHALLING(RSRenderTransition)
MARSHALLING_AND_UNMARSHALLING(RSRenderTransitionEffect)
#ifndef USE_ROSEN_DRAWING
MARSHALLING_AND_UNMARSHALLING(DrawCmdList)
#endif
#undef MARSHALLING_AND_UNMARSHALLING

#define MARSHALLING_AND_UNMARSHALLING(TEMPLATE)                                                    \
    bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE>& val)    \
    {                                                                                              \
        return {};                                                                                 \
    }                                                                                              \
    bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE>& val)        \
    {                                                                                              \
        return {};                                                                                 \
    }

MARSHALLING_AND_UNMARSHALLING(RSRenderCurveAnimation)
MARSHALLING_AND_UNMARSHALLING(RSRenderParticleAnimation)
MARSHALLING_AND_UNMARSHALLING(RSRenderInterpolatingSpringAnimation)
MARSHALLING_AND_UNMARSHALLING(RSRenderKeyframeAnimation)
MARSHALLING_AND_UNMARSHALLING(RSRenderSpringAnimation)
MARSHALLING_AND_UNMARSHALLING(RSRenderPathAnimation)
#undef MARSHALLING_AND_UNMARSHALLING

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSRenderModifier>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSRenderModifier>& val)
{
    return {};
}

#define MARSHALLING_AND_UNMARSHALLING(TEMPLATE)                                                                       \
    template<typename T>                                                                                              \
    bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<T>>& val)                    \
    {                                                                                                                 \
        return parcel.WriteInt16(static_cast<int16_t>(val->GetPropertyType())) && parcel.WriteUint64(val->GetId()) && \
               Marshalling(parcel, val->Get());                                                                       \
    }                                                                                                                 \
    template<typename T>                                                                                              \
    bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<T>>& val)                        \
    {                                                                                                                 \
        PropertyId id = 0;                                                                                            \
        int16_t typeId = 0;                                                                                           \
        if (!parcel.ReadInt16(typeId)) {                                                                              \
            return false;                                                                                             \
        }                                                                                                             \
        RSRenderPropertyType type = static_cast<RSRenderPropertyType>(typeId);                                        \
        if (!parcel.ReadUint64(id)) {                                                                                 \
            return false;                                                                                             \
        }                                                                                                             \
        T value;                                                                                                      \
        if (!Unmarshalling(parcel, value)) {                                                                          \
            return false;                                                                                             \
        }                                                                                                             \
        val.reset(new TEMPLATE<T>(value, id, type));                                                                  \
        return val != nullptr;                                                                                        \
    }

MARSHALLING_AND_UNMARSHALLING(RSRenderProperty)
MARSHALLING_AND_UNMARSHALLING(RSRenderAnimatableProperty)
#undef MARSHALLING_AND_UNMARSHALLING

#define EXPLICIT_INSTANTIATION(TEMPLATE, TYPE)                                                                  \
    template bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<TYPE>>& val); \
    template bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<TYPE>>& val);

#ifndef USE_ROSEN_DRAWING
#define BATCH_EXPLICIT_INSTANTIATION(TEMPLATE)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, bool)                                \
    EXPLICIT_INSTANTIATION(TEMPLATE, float)                               \
    EXPLICIT_INSTANTIATION(TEMPLATE, int)                                 \
    EXPLICIT_INSTANTIATION(TEMPLATE, Color)                               \
    EXPLICIT_INSTANTIATION(TEMPLATE, Gravity)                             \
    EXPLICIT_INSTANTIATION(TEMPLATE, ForegroundColorStrategyType)         \
    EXPLICIT_INSTANTIATION(TEMPLATE, Matrix3f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Quaternion)                          \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSFilter>)           \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSImage>)            \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSMask>)             \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSPath>)             \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSShader>)           \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSLinearGradientBlurPara>)    \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSRenderParticle>)    \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector2f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<uint32_t>)                   \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<Color>)                      \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, RRectT<float>)                       \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<DrawCmdList>)        \
    EXPLICIT_INSTANTIATION(TEMPLATE, SkMatrix)
#else
#define BATCH_EXPLICIT_INSTANTIATION(TEMPLATE)                     \
    EXPLICIT_INSTANTIATION(TEMPLATE, bool)                         \
    EXPLICIT_INSTANTIATION(TEMPLATE, float)                        \
    EXPLICIT_INSTANTIATION(TEMPLATE, int)                          \
    EXPLICIT_INSTANTIATION(TEMPLATE, Color)                        \
    EXPLICIT_INSTANTIATION(TEMPLATE, Gravity)                      \
    EXPLICIT_INSTANTIATION(TEMPLATE, GradientDirection)            \
    EXPLICIT_INSTANTIATION(TEMPLATE, ForegroundColorStrategyType)  \
    EXPLICIT_INSTANTIATION(TEMPLATE, Matrix3f)                     \
    EXPLICIT_INSTANTIATION(TEMPLATE, Quaternion)                   \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSFilter>)    \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSImage>)     \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSMask>)      \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSPath>)      \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSShader>)    \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSLinearGradientBlurPara>)    \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector2f)                     \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<uint32_t>)            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<Color>)               \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4f)                     \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<Drawing::DrawCmdList>) \
    EXPLICIT_INSTANTIATION(TEMPLATE, Drawing::Matrix)
#endif

BATCH_EXPLICIT_INSTANTIATION(RSRenderProperty)

#undef EXPLICIT_INSTANTIATION
#undef BATCH_EXPLICIT_INSTANTIATION

#define EXPLICIT_INSTANTIATION(TEMPLATE, TYPE)                                                                  \
    template bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<TYPE>>& val); \
    template bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<TYPE>>& val);

#define BATCH_EXPLICIT_INSTANTIATION(TEMPLATE)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, float)                               \
    EXPLICIT_INSTANTIATION(TEMPLATE, int)                                 \
    EXPLICIT_INSTANTIATION(TEMPLATE, Color)                               \
    EXPLICIT_INSTANTIATION(TEMPLATE, Matrix3f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Quaternion)                          \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSFilter>)           \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector2f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<Color>)                      \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, RRectT<float>)

BATCH_EXPLICIT_INSTANTIATION(RSRenderAnimatableProperty)

#undef EXPLICIT_INSTANTIATION
#undef BATCH_EXPLICIT_INSTANTIATION

// explicit instantiation
template bool RSMarshallingHelper::Marshalling(
    Parcel& parcel, const std::vector<std::shared_ptr<RSRenderTransitionEffect>>& val);
template bool RSMarshallingHelper::Unmarshalling(
    Parcel& parcel, std::vector<std::shared_ptr<RSRenderTransitionEffect>>& val);

bool RSMarshallingHelper::WriteToParcel(Parcel& parcel, const void* data, size_t size)
{
    return {};
}

const void* RSMarshallingHelper::ReadFromParcel(Parcel& parcel, size_t size)
{
    return {};
}

bool RSMarshallingHelper::SkipFromParcel(Parcel& parcel, size_t size)
{
    return {};
}
} // namespace Rosen
} // namespace OHOS
