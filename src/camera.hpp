#pragma once

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace camera
{
    static constexpr float min_fov = 20.f * M_PI / 180.f;
    static constexpr float max_fov = M_PI;

    static constexpr float near_clipping_plane_dist = 0.1f;
    static constexpr float far_clipping_plane_dist = 10.f;

    float clamp_angle(float);
}

class Camera
{
public:
    Camera(float aspect_ratio_) : aspect_ratio(aspect_ratio_) {}

    glm::mat4 calc_world_to_clip() const;

    float get_fov() const { return fov; }
    void set_fov(float new_fov);

    float get_horizontal_angle() const { return horizontal_angle; }
    void set_horizontal_angle(float);

    float get_vertical_angle() const { return vertical_angle; }
    void set_vertical_angle(float);
private:
    float aspect_ratio;
    float fov = glm::radians(60.f);
    float horizontal_angle = 0.f;
    float vertical_angle = 0.f;
};

glm::mat4 Camera::calc_world_to_clip() const
{
    const glm::vec4 focus =
        glm::rotate(horizontal_angle, glm::vec3(0.f, 1.f, 0.f))
        * glm::rotate(vertical_angle, glm::vec3(1.f, 0.f, 0.f))
        * glm::vec4(0.f, 0.f, 1.f, 1.f);
    const glm::mat4 view = glm::lookAt(
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(focus),
            glm::vec3(0.f, 1.f, 0.f));
    const glm::mat4 projection = glm::perspective(
            fov, aspect_ratio,
            camera::near_clipping_plane_dist, camera::far_clipping_plane_dist);

    return projection * view;
}

void Camera::set_fov(float new_fov)
{
    fov = glm::clamp(new_fov, camera::min_fov, camera::max_fov);
}

void Camera::set_horizontal_angle(float new_angle)
{
    horizontal_angle = camera::clamp_angle(new_angle);
}

void Camera::set_vertical_angle(float new_angle)
{
    vertical_angle = camera::clamp_angle(new_angle);
}

float camera::clamp_angle(float angle)
{
    return glm::mod(angle, 2.f * (float) M_PI);
}
