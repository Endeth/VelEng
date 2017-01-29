
#include "VLight.h"

Vel::VLightSource::VLightColor::VLightColor(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _ambient(ambient), _diffuse(diffuse), _specular(specular)
{
}

Vel::VLightSource::VLightSource(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _colors(ambient, diffuse, specular)
{
}

Vel::VLightSource::VLightSource(const VLightColor & colors) : _colors(colors)
{
}

Vel::VPointLight::VPointLight(const glm::vec3 & position, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _position(position)
{
}

Vel::VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors), _position(position)
{
}

Vel::VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _direction(direction)
{
}

Vel::VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const VLightColor & colors) : VLightSource(colors), _direction(direction)
{
}
