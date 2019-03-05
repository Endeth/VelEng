#include "vCamera.h"

namespace Vel
{
	BaseCamera::BaseCamera() : _position( glm::vec3{ 0.0, 0.0, 0.0 } ), _yaw( 0.0f ), _pitch( 0.0f ), _roll( 0.0f ), _zNear( 0.1f ), _zFar( 1000.f )
	{
	}

	BaseCamera::~BaseCamera()
	{
	}

	void BaseCamera::SetupProjection( const float radFOVY, const float aspectRatio, const float nearD, const float farD )
	{
		_projection = glm::perspective( radFOVY, aspectRatio, nearD, farD );
		_zNear = nearD;
		_zFar = farD;
		_fov = radFOVY;
		_aspectRatio = aspectRatio;
	}

	void BaseCamera::SetupProjection( const int degFOVY, const float aspectRatio, const float nearD, const float farD )
	{
		SetupProjection(glm::radians(float(degFOVY)), aspectRatio, nearD, farD);
	}

	void BaseCamera::Rotate( const float yaw, const float pitch, const float roll )
	{
		_yaw = glm::radians( yaw );
		_pitch = glm::radians( pitch );
		_roll = glm::radians( roll );
		Update();
	}

	const glm::mat4& BaseCamera::GetViewMatrix() const
	{
		return _view;
	}

	const glm::mat4& BaseCamera::GetProjectionMatrix() const
	{
		return _projection;
	}

	void BaseCamera::SetPosition(const glm::vec3 & pos)
	{
		_position = pos;
	}

	const glm::vec3 BaseCamera::GetPosition() const
	{
		return _position;
	}

	void BaseCamera::SetFOV(const float FOV)
	{
		_fov = FOV;
		_projection = glm::perspective(FOV, _aspectRatio, _zNear, _zFar);
	}

	const float BaseCamera::GetFOV() const
	{
		return _fov;
	}

	const float BaseCamera::GetAspectRatio() const
	{
		return _aspectRatio;
	}

	const glm::vec3 BaseCamera::GetDirectionVector() const
	{
		return glm::normalize(_look);
	}

	void FreeCamera::Update()
	{
		glm::mat4 rotation( 1.f ); //= glm::yawPitchRoll( _yaw, _pitch, _roll );
		rotation = glm::rotate( rotation, _yaw, glm::vec3( 0, 1, 0 ) );
		rotation = glm::rotate( rotation, _pitch, glm::vec3( 1, 0, 0 ) );
		//rotation = glm::transpose( rotation );
		//rotation = glm::rotate( rotation, _roll * 0.1f, glm::vec3( 0, 0, 1 ) );
		_position += _translation;

		_look = glm::vec3( rotation * glm::vec4( 0.0f, 0.0f, 1.0f, 0.0f ) );
		_up = glm::vec3( rotation * glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f ) ); // -1 reverts up-down movement
		_right = glm::cross( _look, _up );

		glm::vec3 tgt = _position + _look;
		_view = glm::lookAt( _position, tgt, _up );
		_translation = { 0.0, 0.0, 0.0 }; //TODO smooth movement
	}

	void FreeCamera::Walk(const float dT)
	{
		_translation = (_look * _speed * dT);
		Update();
	}

	void FreeCamera::Strafe(const float dT)
	{
		_translation = (_right * _speed * dT);
		Update();
	}

	void FreeCamera::Lift(const float dT)
	{
		_translation = (_up * _speed * dT);
		Update();
	}

	void FreeCamera::SetTranslation(const glm::vec3 & t)
	{
		_translation = t;
		Update();
	}

	glm::vec3 FreeCamera::GetTranslation() const
	{
		return _translation;
	}

	void FreeCamera::SetSpeed(const float speed)
	{
		_speed = speed;
	}

	const float FreeCamera::GetSpeed() const
	{
		return _speed;
	}

}