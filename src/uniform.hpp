#pragma once

#include <stdexcept>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

template <typename T>
class Uniform
{
public:
    Uniform(const GLuint program_id, const std::string name);

    T get() const;
    void set(T);
private:
    T value;
    GLint location;

    void assign_gl_uniform() const;
};

template <typename T>
Uniform<T>::Uniform(const GLuint program_id, const std::string name)
    : location(glGetUniformLocation(program_id, name.c_str()))
{
    if (location == -1) {
        throw std::runtime_error("Uniform does not exist: " + name);
    }
}

template <typename T>
T Uniform<T>::get() const
{
    return value;
}

template <typename T>
void Uniform<T>::set(T new_value)
{
    value = new_value;
    assign_gl_uniform();
}

template <>
void Uniform<glm::mat4>::assign_gl_uniform() const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
