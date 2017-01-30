#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "VGeo/VGeo.h"

namespace Vel
{

	class VBaseCamera
	{
	public:
		VBaseCamera();
		virtual ~VBaseCamera();

		
		virtual void Update() = 0;
		virtual void Rotate(const float yaw, const float pitch, const float roll);

		const glm::mat4& GetViewMatrix() const;

		void SetupProjection(const float radFOVY, const float aspectRatio, const float nearD = 0.1f, const float farD = 1000.0f);
		void SetupProjection(const int degFOVY, const float aspectRatio, const float nearD = 0.1f, const float farD = 1000.0f);
		const glm::mat4& GetProjectionMatrix() const;

		void SetPosition(const glm::vec3 &pos);
		const glm::vec3 GetPosition() const;

		void SetFOV(const float FOV);
		const float GetFOV() const;
		const float GetAspectRatio() const;

		const glm::vec3 GetDirectionVector() const;

		void CalcFrustumPlanes();

		bool IsPointInFrustum(const glm::vec3 &point);
		bool IsSphereInFrustum(const glm::vec3 &center, const float radius);
		bool IsBoxInFrustum(const glm::vec3 &min, const glm::vec3 &max);
		void GetFrustumPlanes(glm::vec4 planes[6]);

		glm::vec3 FarPts[4];
		glm::vec3 NearPts[4];

	protected:
		float _yaw, _pitch, _roll, _fov, _aspectRatio, _zNear, _zFar;
		static glm::vec3 UP;


		glm::vec3 _look;
		glm::vec3 _up;
		glm::vec3 _right;
		glm::vec3 _position;
		glm::mat4 _view;
		glm::mat4 _projection;

		VPlane _clipingPlanes[6];
	};

	class VFreeCamera : public VBaseCamera
	{
	public:
		VFreeCamera();
		~VFreeCamera();

		void Update();

		void Walk(const float dT);
		void Strafe(const float dT);
		void Lift(const float dT);

		void SetTranslation(const glm::vec3 &t);
		glm::vec3 GetTranslation() const;

		void SetSpeed(const float speed);
		const float GetSpeed() const;

	protected:
		float _speed;
		glm::vec3 _translation;
	};

	class VMirrorCamera : public VBaseCamera
	{
	public:
		VMirrorCamera();
		~VMirrorCamera();


		void SetPosition(const glm::vec3 &position);
		void SetPosition(const glm::vec3 &reflectedCameraPosition, const VPlane &reflectionPlane);
		void SetupProjection(const glm::vec3 &mirrorPosition, GLfloat mirrorWidth, GLfloat mirrorHeight);
	private:
		void SetFOV(const glm::vec3 &mirrorPosition, GLfloat mirrorWidth, GLfloat mirrorHeight);
	};

}