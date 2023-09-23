#include "controller.h"

#include <arde/math/vector.h>

arde_transform_t arde_controller_zoom(const arde_transform_t * transform, float zoom_factor)
{
    arde_transform_t new_transform;
    new_transform.rotation = transform->rotation;
    new_transform.scaling.data[0] = zoom_factor;
    new_transform.scaling.data[1] = zoom_factor;
    new_transform.translation = transform->translation;
    return new_transform;
}
