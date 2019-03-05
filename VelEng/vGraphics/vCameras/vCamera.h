#pragma once

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "vGeo/vGeo.h"

namespace Vel
{

	class BaseCamera //TODO fix camera. also, quaternions?
	{
	public:
		BaseCamera();
		virtual ~BaseCamera();

		virtual void Update() = 0;
		virtual void Rotate( const float yaw, const float pitch, const float roll );

		const glm::mat4& GetViewMatrix() const;

		void SetupProjection( const float radFOVY, const float aspectRatio, const float nearD = 0.1f, const float farD = 1000.0f );
		void SetupProjection( const int degFOVY, const float aspectRatio, const float nearD = 0.1f, const float farD = 1000.0f );
		const glm::mat4& GetProjectionMatrix() const;

		void SetPosition( const glm::vec3 &pos );
		const glm::vec3 GetPosition() const;

		void SetFOV( const float FOV );
		const float GetFOV() const;
		const float GetAspectRatio() const;

		const glm::vec3 GetDirectionVector() const;

	protected:
		float _yaw, _pitch, _roll, _fov, _aspectRatio, _zNear, _zFar;

		glm::vec3 _look = glm::vec3( 0.f );
		glm::vec3 _up = glm::vec3( 0.f );
		glm::vec3 _right = glm::vec3( 0.f );
		glm::vec3 _position = glm::vec3( 0.f );
		glm::mat4 _view = glm::mat4( 1.f );
		glm::mat4 _projection = glm::mat4( 1.f );
	};

	class FreeCamera : public BaseCamera
	{
	public:
		virtual void Update() override;

		void Walk( const float dT );
		void Strafe( const float dT );
		void Lift( const float dT );

		void SetTranslation( const glm::vec3 &t );
		glm::vec3 GetTranslation() const;

		void SetSpeed( const float speed );
		const float GetSpeed() const;

	protected:
		float _speed = 0.05f;
		glm::vec3 _translation = glm::vec3( 0.f );
	};

	/*class VMirrorCamera : public BaseCamera //not implemented right now, also - not needed right now
	{
	public:
		VMirrorCamera();
		~VMirrorCamera();


		void SetPosition(const glm::vec3 &position);
		void SetPosition(const glm::vec3 &reflectedCameraPosition, const VPlane &reflectionPlane);
		void SetupProjection(const glm::vec3 &mirrorPosition, float mirrorWidth, float mirrorHeight);
	private:
		void SetFOV(const glm::vec3 &mirrorPosition, float mirrorWidth, float mirrorHeight);
	};*/

}