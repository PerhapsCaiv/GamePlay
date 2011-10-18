/*
 * BoundingBox.cpp
 */

#include "Base.h"
#include "BoundingBox.h"
#include "BoundingSphere.h"

namespace gameplay
{

BoundingBox::BoundingBox()
{
}

BoundingBox::BoundingBox(const Vector3& min, const Vector3& max)
{
    set(min, max);
}

BoundingBox::BoundingBox(const BoundingBox& copy)
{
    set(copy);
}

BoundingBox::~BoundingBox()
{
}

const BoundingBox& BoundingBox::empty()
{
    static BoundingBox b;
    return b;
}

void BoundingBox::getCorners(Vector3* dst) const
{
    assert(dst);

    // Near face, specified counter-clockwise looking towards the origin from the positive z-axis.
    // Left-top-front.
    dst[0].set(min.x, max.y, max.z);
    // Left-bottom-front.
    dst[1].set(min.x, min.y, max.z);
    // Right-bottom-front.
    dst[2].set(max.x, min.y, max.z);
    // Right-top-front.
    dst[3].set(max.x, max.y, max.z);

    // Far face, specified counter-clockwise looking towards the origin from the negative z-axis.
    // Right-top-back.
    dst[4].set(max.x, max.y, min.z);
    // Right-bottom-back.
    dst[5].set(max.x, min.y, min.z);
    // Left-bottom-back.
    dst[6].set(min.x, min.y, min.z);
    // Left-top-back.
    dst[7].set(min.x, max.y, min.z);
}

void BoundingBox::getCenter(Vector3* dst) const
{
    dst->set(min, max);
    dst->scale(0.5f);
    dst->add(min);
}

bool BoundingBox::intersects(const BoundingSphere& sphere) const
{
    return sphere.intersects(*this);
}

bool BoundingBox::intersects(const BoundingBox& box) const
{
    return ((min.x >= box.min.x && min.x <= max.x) || (box.min.x >= min.x && box.min.x <= max.x)) && ((min.y
        >= box.min.y && min.y <= max.y) || (box.min.y >= min.y && box.min.y <= max.y)) && ((min.z >= box.min.z
        && min.z <= max.z) || (box.min.z >= min.z && box.min.z <= max.z));
}

bool BoundingBox::intersects(const Frustum& frustum) const
{
    // The box must either intersect or be in the positive half-space of all six planes of the frustum.
    return (intersects(frustum.getNear()) != PLANE_INTERSECTS_BACK && intersects(frustum.getFar())
        != PLANE_INTERSECTS_BACK && intersects(frustum.getLeft()) != PLANE_INTERSECTS_BACK && intersects(
        frustum.getRight()) != PLANE_INTERSECTS_BACK && intersects(frustum.getBottom()) != PLANE_INTERSECTS_BACK
        && intersects(frustum.getTop()) != PLANE_INTERSECTS_BACK);
}

float BoundingBox::intersects(const Plane& plane) const
{
    // Calculate the distance from the center of the box to the plane.
    Vector3 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
    float distance = plane.distance(center);

    // Get the extents of the box from its center along each axis.
    float extentX = (max.x - min.x) * 0.5f;
    float extentY = (max.y - min.y) * 0.5f;
    float extentZ = (max.z - min.z) * 0.5f;

    const Vector3& planeNormal = plane.getNormal();
    if (fabsf(distance) <= (fabsf(extentX * planeNormal.x) + fabsf(extentY * planeNormal.y) + fabsf(
        extentZ * planeNormal.z)))
    {
        return PLANE_INTERSECTS_INTERSECTING;
    }

    return (float)(distance > 0.0f) ? PLANE_INTERSECTS_FRONT : PLANE_INTERSECTS_BACK;
}

float BoundingBox::intersects(const Ray& ray) const
{
    // Intermediate calculation variables.
    float dnear = 0.0f;
    float dfar = 0.0f;
    float tmin = 0.0f;
    float tmax = 0.0f;

    const Vector3& origin = ray.getOrigin();
    const Vector3& direction = ray.getDirection();

    // X direction.
    float div = 1.0f / direction.x;
    if (div >= 0.0f)
    {
        tmin = (min.x - origin.x) * div;
        tmax = (max.x - origin.x) * div;
    }
    else
    {
        tmin = (max.x - origin.x) * div;
        tmax = (min.x - origin.x) * div;
    }
    dnear = tmin;
    dfar = tmax;

    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        return RAY_INTERSECTS_NONE;
    }

    // Y direction.
    div = 1.0f / direction.y;
    if (div >= 0.0f)
    {
        tmin = (min.y - origin.y) * div;
        tmax = (max.y - origin.y) * div;
    }
    else
    {
        tmin = (max.y - origin.y) * div;
        tmax = (min.y - origin.y) * div;
    }

    // Update the near and far intersection distances.
    if (tmin > dnear)
    {
        dnear = tmin;
    }
    if (tmax < dfar)
    {
        dfar = tmax;
    }
    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        return RAY_INTERSECTS_NONE;
    }

    // Z direction.
    div = 1.0f / direction.z;
    if (div >= 0.0f)
    {
        tmin = (min.z - origin.z) * div;
        tmax = (max.z - origin.z) * div;
    }
    else
    {
        tmin = (max.z - origin.z) * div;
        tmax = (min.z - origin.z) * div;
    }

    // Update the near and far intersection distances.
    if (tmin > dnear)
    {
        dnear = tmin;
    }
    if (tmax < dfar)
    {
        dfar = tmax;
    }

    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        return RAY_INTERSECTS_NONE;
    }
    // The ray intersects the box (and since the direction of a Ray is normalized, dnear is the distance to the ray).
    return dnear;
}

bool BoundingBox::isEmpty() const
{
    return min.x == max.x && min.y == max.y && min.z == max.z;
}

void BoundingBox::merge(const BoundingBox& box)
{
    // Calculate the new minimum point.
    min.x = fminf(min.x, box.min.x);
    min.y = fminf(min.y, box.min.y);
    min.z = fminf(min.z, box.min.z);

    // Calculate the new maximum point.
    max.x = fmaxf(max.x, box.max.x);
    max.y = fmaxf(max.y, box.max.y);
    max.z = fmaxf(max.z, box.max.z);
}

void BoundingBox::merge(const BoundingSphere& sphere)
{
    const Vector3& center = sphere.center;
    float radius = sphere.radius;

    // Calculate the new minimum point for the merged bounding box.
    min.x = fminf(min.x, center.x - radius);
    min.y = fminf(min.y, center.y - radius);
    min.z = fminf(min.z, center.z - radius);

    // Calculate the new maximum point for the merged bounding box.
    max.x = fmaxf(max.x, center.x + radius);
    max.y = fmaxf(max.y, center.y + radius);
    max.z = fmaxf(max.z, center.z + radius);
}

void BoundingBox::set(const Vector3& min, const Vector3& max)
{
    this->min = min;
    this->max = max;
}

void updateMinMax(Vector3* point, Vector3* min, Vector3* max)
{
    // Leftmost point.
    if (point->x < min->x)
    {
        min->x = point->x;
    }

    // Rightmost point.
    if (point->x > max->x)
    {
        max->x = point->x;
    }

    // Lowest point.
    if (point->y < min->y)
    {
        min->y = point->y;
    }

    // Highest point.
    if (point->y > max->y)
    {
        max->y = point->y;
    }

    // Farthest point.
    if (point->z < min->z)
    {
        min->z = point->z;
    }

    // Nearest point.
    if (point->z > max->z)
    {
        max->z = point->z;
    }
}

void BoundingBox::set(const BoundingBox& box)
{
    min = box.min;
    max = box.max;
}

void BoundingBox::set(const BoundingSphere& sphere)
{
    std::vector<int> v;
    v.push_back(0);
    std::vector<int> v2 = v;

    const Vector3& center = sphere.center;
    float radius = sphere.radius;

    // Calculate the minimum point for the box.
    min.x = center.x - radius;
    min.y = center.y - radius;
    min.z = center.z - radius;

    // Calculate the maximum point for the box.
    max.x = center.x + radius;
    max.y = center.y + radius;
    max.z = center.z + radius;
}

void BoundingBox::transform(const Matrix& matrix)
{
    // Calculate the corners.
    Vector3 corners[8];
    getCorners(corners);

    // Transform the corners, recalculating the min and max points along the way.
    matrix.transformPoint(&corners[0]);
    Vector3 newMin = corners[0];
    Vector3 newMax = corners[0];
    for (int i = 1; i < 8; i++)
    {
        matrix.transformPoint(&corners[i]);
        updateMinMax(&corners[i], &newMin, &newMax);
    }
    this->min.x = newMin.x;
    this->min.y = newMin.y;
    this->min.z = newMin.z;
    this->max.x = newMax.x;
    this->max.y = newMax.y;
    this->max.z = newMax.z;
}

}
