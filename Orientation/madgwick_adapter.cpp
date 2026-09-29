#include "madgwick_adapter.h"

#include "Madgwick.h"

#include <algorithm>
#include <cmath>

namespace
{

constexpr double correction_gain = 0.1;
constexpr double bias_gain = 0.001;
const madgwick::Vector3 gravity_reference(0.0, 0.0, 1.0);

madgwick::Filter orientation_filter(correction_gain, bias_gain);

bool vector_is_finite(const double vector[3])
{
    return vector != nullptr && std::isfinite(vector[0]) && std::isfinite(vector[1]) &&
           std::isfinite(vector[2]);
}

void quaternion_to_euler(const madgwick::Quaternion &orientation, double euler_angles[3])
{
    const double w = orientation.w();
    const double x = orientation.x();
    const double y = orientation.y();
    const double z = orientation.z();

    const double sin_pitch = std::clamp(2.0 * (w * y - z * x), -1.0, 1.0);

    euler_angles[0] = std::atan2(2.0 * (w * x + y * z), 1.0 - 2.0 * (x * x + y * y));
    euler_angles[1] = std::asin(sin_pitch);
    euler_angles[2] = std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
}

} // namespace

extern "C" void madgwick_adapter_reset(void)
{
    orientation_filter.reset();
}

extern "C" madgwick_update_status madgwick_adapter_update(const double acceleration[3],
                                                          const double angular_velocity[3],
                                                          double sample_period_s,
                                                          double euler_angles[3])
{
    if (!vector_is_finite(acceleration) || !vector_is_finite(angular_velocity) ||
        euler_angles == nullptr || !std::isfinite(sample_period_s) || sample_period_s <= 0.0)
    {
        return MADGWICK_UPDATE_INVALID_INPUT;
    }

    const madgwick::Vector3 acceleration_vector(acceleration[0], acceleration[1], acceleration[2]);
    if (acceleration_vector.squaredNorm() == 0.0)
    {
        return MADGWICK_UPDATE_INVALID_INPUT;
    }

    const madgwick::DirectionObservation gravity_observation{gravity_reference,
                                                             acceleration_vector};
    const madgwick::Vector3 angular_velocity_vector(angular_velocity[0], angular_velocity[1],
                                                    angular_velocity[2]);

    try
    {
        orientation_filter.update(gravity_observation, angular_velocity_vector, sample_period_s);
        quaternion_to_euler(orientation_filter.orientation(), euler_angles);
    }
    catch (...)
    {
        orientation_filter.reset();
        return MADGWICK_UPDATE_INVALID_INPUT;
    }

    return MADGWICK_UPDATE_OK;
}
