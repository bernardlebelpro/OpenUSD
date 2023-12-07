//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdGeom/blundaCamera.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdGeomBlundaCamera,
        TfType::Bases< UsdGeomCamera > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("Camera")
    // to find TfType<UsdGeomCamera>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdGeomBlundaCamera>("BlundaCamera");
}

/* virtual */
UsdGeomBlundaCamera::~UsdGeomBlundaCamera()
{
}

/* static */
UsdGeomBlundaCamera
UsdGeomBlundaCamera::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdGeomBlundaCamera();
    }
    return UsdGeomBlundaCamera(stage->GetPrimAtPath(path));
}

/* static */
UsdGeomBlundaCamera
UsdGeomBlundaCamera::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("BlundaCamera");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdGeomBlundaCamera();
    }
    return UsdGeomBlundaCamera(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdGeomBlundaCamera::_GetSchemaKind() const
{
    return UsdGeomBlundaCamera::schemaKind;
}

/* static */
const TfType &
UsdGeomBlundaCamera::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdGeomBlundaCamera>();
    return tfType;
}

/* static */
bool 
UsdGeomBlundaCamera::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdGeomBlundaCamera::_GetTfType() const
{
    return _GetStaticTfType();
}

// ----------------------------------------------------------------------------
// ATTRIBUTES
// ----------------------------------------------------------------------------

UsdAttribute
UsdGeomBlundaCamera::GetHohohoAttr() const
{
    return GetPrim().GetAttribute(UsdGeomTokens->hohoho);
}

UsdAttribute
UsdGeomBlundaCamera::CreateHohohoAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdGeomTokens->hohoho,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

namespace {
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector& left,const TfTokenVector& right)
{
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
}
}

/*static*/
const TfTokenVector&
UsdGeomBlundaCamera::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdGeomTokens->hohoho,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomCamera::GetSchemaAttributeNames(true),
            localNames);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

PXR_NAMESPACE_OPEN_SCOPE

// ----------------------------------------------------------------------------
// HELPERS
// ----------------------------------------------------------------------------

template<class T>
static boost::optional<T> _GetValue(const UsdPrim &prim,
                                    const TfToken &name,
                                    const UsdTimeCode &time)
{
    const UsdAttribute attr = prim.GetAttribute(name);
    if (!attr) {
        TF_WARN("%s attribute on prim %s missing.",
                name.GetText(), prim.GetPath().GetText());
        return boost::none;
    }
    
    T value;
    if (!attr.Get(&value, time)) {
        TF_WARN("Failed to extract value from attribute %s at <%s>.",
                name.GetText(), attr.GetPath().GetText());
        return boost::none;
    }

    return value;
}

static
GfVec2f
_Range1fToVec2f(const GfRange1f &range)
{
    return GfVec2f(range.GetMin(), range.GetMax());
}

static
TfToken
_ProjectionToToken(GfBlundaCamera::Projection projection)
{
    switch(projection) {
    case GfBlundaCamera::Perspective:
        return UsdGeomTokens->perspective;
    case GfBlundaCamera::Orthographic:
        return UsdGeomTokens->orthographic;
    default:
        TF_WARN("Unknown projection type %d", projection);
        return TfToken();
    }
}

static
GfBlundaCamera::Projection
_TokenToProjection(const TfToken &token)
{
    if (token == UsdGeomTokens->orthographic) {
        return GfBlundaCamera::Orthographic;
    }

    if (token != UsdGeomTokens->perspective) {
        TF_WARN("Unknown projection type %s", token.GetText());
    }

    return GfBlundaCamera::Perspective;
}

static
VtArray<GfVec4f>
_VectorVec4fToVtArray(const std::vector<GfVec4f> &vec)
{
    VtArray<GfVec4f> result;
    result.assign(vec.begin(), vec.end());
    return result;
}

// ----------------------------------------------------------------------------
// BASE CLASS OVERRIDES
// ----------------------------------------------------------------------------

GfBlundaCamera
UsdGeomBlundaCamera::GetCamera(const UsdTimeCode &time) const
{
    GfCamera baseCamera = UsdGeomCamera::GetCamera(time);
    GfBlundaCamera camera;
    camera.SetTransform(baseCamera.GetTransform());
    camera.SetHorizontalAperture(baseCamera.GetHorizontalAperture());
    camera.SetVerticalAperture(baseCamera.GetVerticalAperture());
    camera.SetHorizontalApertureOffset(baseCamera.GetHorizontalApertureOffset());
    camera.SetVerticalApertureOffset(baseCamera.GetVerticalApertureOffset());
    camera.SetFocalLength(baseCamera.GetFocalLength());
    camera.SetClippingRange(baseCamera.GetClippingRange());
    camera.SetClippingPlanes(baseCamera.GetClippingPlanes());
    camera.SetFStop(baseCamera.GetFStop());
    camera.SetFocusDistance(baseCamera.GetFocusDistance());

    if (const boost::optional<TfToken> projection = _GetValue<TfToken>(
            GetPrim(), UsdGeomTokens->projection, time)) {
        camera.SetProjection(_TokenToProjection(*projection));
    }

    if (const boost::optional<float> hohoho = _GetValue<float>(
            GetPrim(), UsdGeomTokens->hohoho, time)) {
        camera.SetHohoho(*hohoho);
    }

    return camera;
}

void
UsdGeomBlundaCamera::SetFromCamera(const GfBlundaCamera &camera, const UsdTimeCode &time)
{
    const GfMatrix4d parentToWorldInverse =
        ComputeParentToWorldTransform(time).GetInverse();

    const GfMatrix4d camMatrix = camera.GetTransform() * parentToWorldInverse;

    UsdGeomXformOp xformOp = MakeMatrixXform();
    if (!xformOp) {
        // The returned xformOp may be invalid if there are xform op
        // opinions in the composed layer stack stronger than that of
        // the current edit target.
        return;
    }
    xformOp.Set(camMatrix, time);

    GetProjectionAttr().Set(_ProjectionToToken(camera.GetProjection()), time);
    GetHorizontalApertureAttr().Set(camera.GetHorizontalAperture(), time);
    GetVerticalApertureAttr().Set(camera.GetVerticalAperture(), time);
    GetHorizontalApertureOffsetAttr().Set(
        camera.GetHorizontalApertureOffset(), time);
    GetVerticalApertureOffsetAttr().Set(
        camera.GetVerticalApertureOffset(), time);
    GetFocalLengthAttr().Set(camera.GetFocalLength(), time);
    GetClippingRangeAttr().Set(
        _Range1fToVec2f(camera.GetClippingRange()), time);

    GetClippingPlanesAttr().Set(
        _VectorVec4fToVtArray(camera.GetClippingPlanes()), time);

    GetFStopAttr().Set(camera.GetFStop(), time);
    GetFocusDistanceAttr().Set(camera.GetFocusDistance(), time);
    GetHohohoAttr().Set(camera.GetHohoho(), time);
}

PXR_NAMESPACE_CLOSE_SCOPE
