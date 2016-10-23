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
    static constexpr float far_clipping_plane_dist = 1000.f;

    static const glm::vec3 up(0.f, 1.f, 0.f);
}

class Camera
{
public:
    Camera(float aspect_ratio_) : aspect_ratio(aspect_ratio_) {}

    glm::mat4 calc_world_to_clip() const;

    float get_fov() const { return fov; }
    void set_fov(float new_fov);

    float get_horizontal_angle() const { return horizontal_angle; }
    float get_vertical_angle() const { return vertical_angle; }
    void look(float horizontal_angle, float vertical_angle);

    glm::vec3 get_position() const { return position; }
    void set_position(glm::vec3 new_position) { position = new_position; }
    void move(float distance);
    void move_right(float distance);
private:
    float aspect_ratio;
    float fov = glm::radians(60.f);
    float horizontal_angle = 0.f;
    float vertical_angle = 0.f;

    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 direction_right = glm::vec3(1.f, 0.f, 0.f);
};

glm::mat4 Camera::calc_world_to_clip() const
{
    const glm::vec3 focus = position + direction;
    const glm::mat4 view = glm::lookAt(position, focus, camera::up);
    const glm::mat4 projection = glm::perspective(
            fov, aspect_ratio,
            camera::near_clipping_plane_dist, camera::far_clipping_plane_dist);

    return projection * view;
}

void Camera::set_fov(const float new_fov)
{
    fov = glm::clamp(new_fov, camera::min_fov, camera::max_fov);
}

void Camera::look(
        const float new_horizontal_angle, const float new_vertical_angle)
{
    horizontal_angle = glm::mod(new_horizontal_angle, glm::radians(360.f));
    vertical_angle = glm::clamp(
            new_vertical_angle, glm::radians(-90.f), glm::radians(90.f));

    const glm::mat4 rotation =
        glm::rotate(horizontal_angle, glm::vec3(0.f, 1.f, 0.f))
        * glm::rotate(vertical_angle, glm::vec3(1.f, 0.f, 0.f));
    direction = glm::vec3(rotation * glm::vec4(0.f, 0.f, -1.f, 1.f));
    direction_right = glm::vec3(rotation * glm::vec4(1.f, 0.f, 0.f, 1.f));
}

void Camera::move(const float distance)
{
    position += distance * direction;
}

void Camera::move_right(const float distance)
{
    position += distance * direction_right;
}
