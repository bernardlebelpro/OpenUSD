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

#include "pxr/pxr.h"
#include "pxr/base/gf/camera.h"
#include "pxr/base/gf/blundaCamera.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/tf/enum.h"
#include "pxr/base/tf/registryManager.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfEnum)
{
    TF_ADD_ENUM_NAME(GfBlundaCamera::Perspective,   "perspective");
    TF_ADD_ENUM_NAME(GfBlundaCamera::Orthographic,  "orthographic");
    TF_ADD_ENUM_NAME(GfBlundaCamera::FOVHorizontal, "FOVHorizontal");
    TF_ADD_ENUM_NAME(GfBlundaCamera::FOVVertical,   "FOVVertical");
}

// Horizontal and vertical aperture is in mm whereas most stuff is in cm.
const double
GfBlundaCamera::APERTURE_UNIT = 0.1;

// Focal length is in mm whereas most stuff is in cm.
const double
GfBlundaCamera::FOCAL_LENGTH_UNIT = 0.1;

// The default filmback size is based on a 35mm spherical
// projector aperture (0.825 x 0.602 inches, converted to
// mm). Note this is slightly different than SMPTE195-2000,
// which specifies 20.96mm (not 20.955mm) and 0.825"  Also
// note that 35mm spherical and anamorphic projector aperture
// widths are the same. Lastly, we use projection aperture
// instead of camera aperture since that's what we film out
// to, and for anyone who cares, 35mm still film has a different
// size, and we don't use that at all with our virtual movie
// camera.
const double
GfBlundaCamera::DEFAULT_HORIZONTAL_APERTURE = (
    0.825 * 2.54 / GfBlundaCamera::APERTURE_UNIT);
const double
GfBlundaCamera::DEFAULT_VERTICAL_APERTURE = (
    0.602 * 2.54 / GfBlundaCamera::APERTURE_UNIT);

GfBlundaCamera::GfBlundaCamera(
    const GfMatrix4d &transform, GfBlundaCamera::Projection projection,
    float horizontalAperture, float verticalAperture,
    float horizontalApertureOffset, float verticalApertureOffset,
    float focalLength,
    const GfRange1f &clippingRange,
    const std::vector<GfVec4f>&clippingPlanes,
    float fStop,
    float focusDistance,
    float hohoho) :
    _transform(transform), _projection(projection),
    _horizontalAperture(horizontalAperture),
    _verticalAperture(verticalAperture),
    _horizontalApertureOffset(horizontalApertureOffset),
    _verticalApertureOffset(verticalApertureOffset),
    _focalLength(focalLength),
    _clippingRange(clippingRange),
    _clippingPlanes(clippingPlanes),
    _fStop(fStop),
    _focusDistance(focusDistance),
    _hohoho(hohoho)
{
}

void
GfBlundaCamera::SetTransform(const GfMatrix4d &val) {
    _transform = val;
}

void
GfBlundaCamera::SetProjection(const GfBlundaCamera::Projection &val) {
    _projection = val;
}

void
GfBlundaCamera::SetHorizontalAperture(const float val) {
    _horizontalAperture = val;
}

void
GfBlundaCamera::SetVerticalAperture(const float val) {
    _verticalAperture = val;
}

void
GfBlundaCamera::SetHorizontalApertureOffset(const float val) {
    _horizontalApertureOffset = val;
}

void
GfBlundaCamera::SetVerticalApertureOffset(const float val) {
    _verticalApertureOffset = val;
}

void
GfBlundaCamera::SetFocalLength(const float val) {
    _focalLength = val;
}

void
GfBlundaCamera::SetPerspectiveFromAspectRatioAndFieldOfView(
    float aspectRatio,
    float fieldOfView,
    GfBlundaCamera::FOVDirection direction,
    float horizontalAperture)
{
    // Perspective
    _projection = Perspective;

    // Set the vertical and horizontal aperture to achieve the aspect ratio
    _horizontalAperture = horizontalAperture;
    _verticalAperture =   horizontalAperture /
        (aspectRatio != 0.0 ? aspectRatio : 1.0);

    // Pick the right dimension based on the direction parameter
    const float aperture =
        (direction == GfBlundaCamera::FOVHorizontal) ?
                           _horizontalAperture : _verticalAperture;
    // Compute tangent for field of view
    const float tanValue = tan(0.5 * GfDegreesToRadians(fieldOfView));
    
    if (tanValue == 0) {
        // To avoid division by zero, just set default value
        _focalLength = 50.0;
        return;
    }
    
    // Do the math for the focal length.
    _focalLength =
        aperture * GfBlundaCamera::APERTURE_UNIT /
        ( 2 * tanValue) / GfBlundaCamera::FOCAL_LENGTH_UNIT;
}

void
GfBlundaCamera::SetOrthographicFromAspectRatioAndSize(
    float aspectRatio, float orthographicSize, FOVDirection direction)
{
    // Orthographic
    _projection = Orthographic;

    // Not used for orthographic cameras, but set to sane values nonetheless
    _focalLength = 50.0;

    // Set horizontal and vertial aperture
    if (direction == FOVHorizontal) {
        // We are given the width, determine height by dividing by aspect ratio
        _horizontalAperture = orthographicSize / GfBlundaCamera::APERTURE_UNIT;
        if (aspectRatio > 0.0) {
            _verticalAperture = _horizontalAperture / aspectRatio;
        } else {
            _verticalAperture = _horizontalAperture;
        }
    } else {
        // We are given the height, determine the width by multiplying
        _verticalAperture = orthographicSize / GfBlundaCamera::APERTURE_UNIT;
        _horizontalAperture = _verticalAperture * aspectRatio;
    }
}

void
GfBlundaCamera::SetFromViewAndProjectionMatrix(
    const GfMatrix4d &viewMatrix, const GfMatrix4d &projMatrix,
    const float focalLength)
{
    _transform = viewMatrix.GetInverse();

    _focalLength = focalLength;

    if (projMatrix[2][3] < -0.5) {
        // Use !(a<b) rather than a>=b so that NaN is caught.
        if (!(fabs(projMatrix[2][3] - (-1.0)) < 1e-6)) {
            TF_WARN("GfBlundaCamera: Given projection matrix does not appear to be "
                    "valid perspective matrix.");
        }

        _projection = Perspective;

        const float apertureBase =
            2.0f * focalLength * (FOCAL_LENGTH_UNIT / APERTURE_UNIT);

        _horizontalAperture =
            apertureBase / projMatrix[0][0];
        _verticalAperture =
            apertureBase / projMatrix[1][1];
        _horizontalApertureOffset =
            0.5 * _horizontalAperture * projMatrix[2][0];
        _verticalApertureOffset =
            0.5 * _verticalAperture * projMatrix[2][1];
        _clippingRange = GfRange1f(
            projMatrix[3][2] / (projMatrix[2][2] - 1.0),
            projMatrix[3][2] / (projMatrix[2][2] + 1.0));
    } else {
        if (!(fabs(projMatrix[2][3]) < 1e-6)) {
            TF_WARN("GfBlundaCamera: Given projection matrix does not appear to be "
                    "valid orthographic matrix.");
        }

        _projection = Orthographic;
        _horizontalAperture =
            (2.0 / APERTURE_UNIT) / projMatrix[0][0];
        _verticalAperture =
            (2.0 / APERTURE_UNIT) / projMatrix[1][1];
        _horizontalApertureOffset =
            -0.5 * (_horizontalAperture) * projMatrix[3][0];
        _verticalApertureOffset =
            -0.5 * (_verticalAperture) * projMatrix[3][1];
        const double nearMinusFarHalf =
            1.0 / projMatrix[2][2];
        const double nearPlusFarHalf =
            nearMinusFarHalf * projMatrix[3][2];
        _clippingRange = GfRange1f(
            nearPlusFarHalf + nearMinusFarHalf,
            nearPlusFarHalf - nearMinusFarHalf);
    }
}

void
GfBlundaCamera::SetClippingRange(const GfRange1f &val) {
    _clippingRange = val;
}

void
GfBlundaCamera::SetClippingPlanes(const std::vector<GfVec4f> &val) {
    _clippingPlanes = val;
}

void
GfBlundaCamera::SetFStop(const float val) {
    _fStop = val;
}

void
GfBlundaCamera::SetFocusDistance(const float val) {
    _focusDistance = val;
}

void
GfBlundaCamera::SetHohoho(const float val) {
    _hohoho = val;
}

GfMatrix4d
GfBlundaCamera::GetTransform() const {
    return _transform;
}

GfBlundaCamera::Projection
GfBlundaCamera::GetProjection() const {
    return _projection;
}

float
GfBlundaCamera::GetHorizontalAperture() const {
    return _horizontalAperture;
}

float
GfBlundaCamera::GetVerticalAperture() const {
    return _verticalAperture;
}

float
GfBlundaCamera::GetHorizontalApertureOffset() const {
    return _horizontalApertureOffset;
}

float
GfBlundaCamera::GetVerticalApertureOffset() const {
    return _verticalApertureOffset;
}

float
GfBlundaCamera::GetAspectRatio() const {
    return (_verticalAperture == 0.0)
               ? 0.0
               : _horizontalAperture / _verticalAperture;
}

float
GfBlundaCamera::GetFocalLength() const {
    return _focalLength;
}

float
GfBlundaCamera::GetFieldOfView(FOVDirection direction) const {
    // Pick the right aperture based on direction
    const float aperture = 
        (direction == FOVHorizontal) ? _horizontalAperture
                                     : _verticalAperture;
    
    // Do the math
    const float fovRAD = 2 * atan(
        (aperture * GfBlundaCamera::APERTURE_UNIT) /
        (2 * _focalLength * GfBlundaCamera::FOCAL_LENGTH_UNIT));

    return GfRadiansToDegrees(fovRAD);
}

GfRange1f
GfBlundaCamera::GetClippingRange() const {
    return _clippingRange;
}

const std::vector<GfVec4f> &
GfBlundaCamera::GetClippingPlanes() const {
    return _clippingPlanes;
}

GfFrustum
GfBlundaCamera::GetFrustum() const {

    const GfVec2d max(_horizontalAperture / 2,
                      _verticalAperture / 2);
    GfRange2d window(-max, max);

    // Apply the aperture offset
    const GfVec2d offsetVec(
        _horizontalApertureOffset, _verticalApertureOffset);
    window += GfRange2d(offsetVec, offsetVec);

    // Up to now, all computations were done in mm, convert to cm.
    window *= GfBlundaCamera::APERTURE_UNIT;

    if (_projection != Orthographic && _focalLength != 0) {
        window /= _focalLength * GfBlundaCamera::FOCAL_LENGTH_UNIT;
    }

    const GfRange1d clippingRange(_clippingRange.GetMin(),
                                  _clippingRange.GetMax());
    
    const GfFrustum::ProjectionType projection = _projection == Orthographic
        ? GfFrustum::Orthographic
        : GfFrustum::Perspective;

    return GfFrustum(_transform, window, clippingRange, projection);
}

float
GfBlundaCamera::GetFStop() const {
    return _fStop;
}

float
GfBlundaCamera::GetFocusDistance() const {
    return _focusDistance;
}

float
GfBlundaCamera::GetHohoho() const {
    return _hohoho;
}

bool
GfBlundaCamera::operator==(const GfCamera& other) const
{
    return
        _transform == other.GetTransform() &&
        _projection == other.GetProjection() &&
        _horizontalAperture == other.GetHorizontalAperture() &&
        _verticalAperture == other.GetVerticalAperture() &&
        _horizontalApertureOffset == other.GetHorizontalApertureOffset() &&
        _verticalApertureOffset == other.GetVerticalApertureOffset() &&
        _focalLength == other.GetFocalLength() &&
        _clippingRange == other.GetClippingRange() &&
        _clippingPlanes == other.GetClippingPlanes() &&
        _fStop == other.GetFStop() &&
        _focusDistance == other.GetFocusDistance();
}

bool
GfBlundaCamera::operator==(const GfBlundaCamera& other) const
{
    return
        _transform == other._transform &&
        _projection == other._projection && 
        _horizontalAperture == other._horizontalAperture && 
        _verticalAperture == other._verticalAperture && 
        _horizontalApertureOffset == other._horizontalApertureOffset && 
        _verticalApertureOffset == other._verticalApertureOffset && 
        _focalLength == other._focalLength && 
        _clippingRange == other._clippingRange && 
        _clippingPlanes == other._clippingPlanes &&
        _fStop == other._fStop &&
        _focusDistance == other._focusDistance &&
        _hohoho == other._hohoho;
}

bool
GfBlundaCamera::operator!=(const GfCamera& other) const
{
    return !(*this == other);
}

bool
GfBlundaCamera::operator!=(const GfBlundaCamera& other) const
{
    return !(*this == other);
}

PXR_NAMESPACE_CLOSE_SCOPE
