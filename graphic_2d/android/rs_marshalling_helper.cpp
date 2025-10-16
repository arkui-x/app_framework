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
#include "securec.h"
#include "animation/rs_particle_ripple_field.h"
#include "animation/rs_particle_velocity_field.h"
#include "animation/rs_render_curve_animation.h"
#include "animation/rs_render_interpolating_spring_animation.h"
#include "animation/rs_render_keyframe_animation.h"
#include "animation/rs_render_particle.h"
#include "animation/rs_render_path_animation.h"
#include "animation/rs_render_spring_animation.h"
#include "animation/rs_render_transition.h"
#include "common/rs_color.h"
#include "common/rs_common_def.h"
#include "common/rs_matrix3.h"
#include "common/rs_vector4.h"
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

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<Drawing::Image>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Drawing::Image>& val, void*& imagepixelAddr)
{
    return {};
}

bool RSMarshallingHelper::SkipImage(Parcel& parcel)
{
    return {};
}

// RSShader
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

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSLinearGradientBlurPara>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSLinearGradientBlurPara>& val)
{
    return {};
}

// MotionBlurPara
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<MotionBlurParam>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<MotionBlurParam>& val)
{
    return {};
}

// MagnifierPara
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSMagnifierParams>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSMagnifierParams>& val)
{
    return {};
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<EmitterUpdater>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<EmitterUpdater>& val)
{
    return {};
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::vector<std::shared_ptr<EmitterUpdater>>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::vector<std::shared_ptr<EmitterUpdater>>& val)
{
    return {};
}

// ParticleNoiseFields
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<ParticleNoiseFields>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<ParticleNoiseFields>& val)
{
    return {};
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<ParticleRippleFields>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<ParticleRippleFields>& val)
{
    return {};
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<ParticleVelocityFields>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<ParticleVelocityFields>& val)
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
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSPath>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSPath>& val)
{
    return {};
}

// RSMask
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSMask>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSMask>& val)
{
    return {};
}

// RSNGRenderFilterBase
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSNGRenderFilterBase>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSNGRenderFilterBase>& val)
{
    return {};
}

// RSNGRenderMaskBase
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSNGRenderMaskBase>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSNGRenderMaskBase>& val)
{
    return {};
}

// RSNGRenderShaderBase
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSNGRenderShaderBase>& val)
{
    return {};
}
bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSNGRenderShaderBase>& val)
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

// Drawing::DrawCmdList
bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<Drawing::DrawCmdList>& val,
    int32_t recordCmdDepth)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<Drawing::DrawCmdList>& val)
{
    return {};
}

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

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<ModifierNG::RSRenderModifier>& val)
{
    return {};
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<ModifierNG::RSRenderModifier>& val)
{
    return {};
}

#define MARSHALLING_AND_UNMARSHALLING(TEMPLATE)                                                    \
    template<typename T>                                                                           \
    bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<T>>& val) \
    {                                                                                              \
        return parcel.WriteUint64(val->GetId()) && Marshalling(parcel, val->Get());                \
    }                                                                                              \
    template<typename T>                                                                           \
    bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<T>>& val)     \
    {                                                                                              \
        PropertyId id = 0;                                                                         \
        if (!RSMarshallingHelper::UnmarshallingPidPlusId(parcel, id)) {                            \
            ROSEN_LOGE("RSMarshallingHelper::Unmarshalling ReadUint64 failed");                    \
            return false;                                                                          \
        }                                                                                          \
        T value;                                                                                   \
        if (!Unmarshalling(parcel, value)) {                                                       \
            ROSEN_LOGE("RSMarshallingHelper::Unmarshalling Unmarshalling failed");                 \
            return false;                                                                          \
        }                                                                                          \
        val.reset(new TEMPLATE<T>(value, id));                                                     \
        return val != nullptr;                                                                     \
    }

MARSHALLING_AND_UNMARSHALLING(RSRenderProperty)
MARSHALLING_AND_UNMARSHALLING(RSRenderAnimatableProperty)
#undef MARSHALLING_AND_UNMARSHALLING

#define EXPLICIT_INSTANTIATION(TEMPLATE, TYPE)                                                                  \
    template bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<TYPE>>& val); \
    template bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<TYPE>>& val);
#define BATCH_EXPLICIT_INSTANTIATION(TEMPLATE)                                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, bool)                                               \
    EXPLICIT_INSTANTIATION(TEMPLATE, float)                                              \
    EXPLICIT_INSTANTIATION(TEMPLATE, int)                                                \
    EXPLICIT_INSTANTIATION(TEMPLATE, Color)                                              \
    EXPLICIT_INSTANTIATION(TEMPLATE, RSDynamicBrightnessPara)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, RSWaterRipplePara)                                  \
    EXPLICIT_INSTANTIATION(TEMPLATE, RSFlyOutPara)                                       \
    EXPLICIT_INSTANTIATION(TEMPLATE, Gravity)                                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, ForegroundColorStrategyType)                        \
    EXPLICIT_INSTANTIATION(TEMPLATE, Matrix3f)                                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, Quaternion)                                         \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSNGRenderFilterBase>)              \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSNGRenderMaskBase>)                \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSNGRenderShaderBase>)              \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSImage>)                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSMask>)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSPath>)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSShader>)                          \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSLinearGradientBlurPara>)          \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<MotionBlurParam>)                   \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<RSMagnifierParams>)                 \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<EmitterUpdater>)                    \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::vector<std::shared_ptr<EmitterUpdater>>)       \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<ParticleNoiseFields>)               \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<ParticleRippleFields>)              \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<ParticleVelocityFields>)            \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<std::vector<ParticleRenderParams>>) \
    EXPLICIT_INSTANTIATION(TEMPLATE, RSRenderParticleVector)                             \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector2f)                                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector3f)                                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<uint32_t>)                                  \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::vector<float>)                                 \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::vector<Vector2f>)                              \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<Media::PixelMap>)                   \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<Color>)                                     \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4f)                                           \
    EXPLICIT_INSTANTIATION(TEMPLATE, RRectT<float>)                                      \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::shared_ptr<Drawing::DrawCmdList>)              \
    EXPLICIT_INSTANTIATION(TEMPLATE, Drawing::Matrix)

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
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector2f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector3f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4<Color>)                      \
    EXPLICIT_INSTANTIATION(TEMPLATE, Vector4f)                            \
    EXPLICIT_INSTANTIATION(TEMPLATE, std::vector<float>)                  \
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

const void* RSMarshallingHelper::ReadFromParcel(Parcel& parcel, size_t size, bool& isMalloc)
{
    return {};
}

bool RSMarshallingHelper::SkipFromParcel(Parcel& parcel, size_t size)
{
    return {};
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const std::shared_ptr<RSRenderPropertyBase>& val)
{
    return true;
}

bool RSMarshallingHelper::CheckReadPosition(Parcel& parcel)
{
    return true;
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, std::shared_ptr<RSRenderPropertyBase>& val)
{
    return true;
}

bool RSMarshallingHelper::UnmarshallingPidPlusId(Parcel& parcel, uint64_t& val)
{
    return true;
}

bool RSMarshallingHelper::UnmarshallingPidPlusIdNoChangeIfZero(Parcel& parcel, uint64_t& val)
{
    return true;
}

bool RSMarshallingHelper::Marshalling(Parcel& parcel, const RSRenderParticleVector& val)
{
    return false;
}

bool RSMarshallingHelper::Unmarshalling(Parcel& parcel, RSRenderParticleVector& val)
{
    return false;
}
} // namespace Rosen
} // namespace OHOS
