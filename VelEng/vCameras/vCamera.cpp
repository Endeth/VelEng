#include "external/glm/gtx/euler_angles.hpp"

#include "vCamera.h"

namespace Vel
{

	glm::vec3 BaseCamera::UP = glm::vec3(0.0f, 1.0f, 0.0f);

	BaseCamera::BaseCamera() : _position(glm::vec3{0.0,3.0,0.0}), _yaw(0.0f), _pitch(0.0f), _roll(0.0f)
	{
		_zNear = 0.1f;
		_zFar = 1000.0f;
	}

	BaseCamera::~BaseCamera()
	{
	}

	void BaseCamera::SetupProjection(const float radFOVY, const float aspectRatio, const float nearD, const float farD)
	{
		_projection = glm::perspective(radFOVY, aspectRatio, nearD, farD);
		_zNear = nearD;
		_zFar = farD;
		_fov = radFOVY;
		_aspectRatio = aspectRatio;
	}

	void BaseCamera::SetupProjection(const int degFOVY, const float aspectRatio, const float nearD, const float farD)
	{
		SetupProjection(glm::radians(float(degFOVY)), aspectRatio, nearD, farD);
	}

	void BaseCamera::Rotate(const float yaw, const float pitch, const float roll)
	{
		_yaw = glm::radians(yaw);
		_pitch = glm::radians(pitch);
		_roll = glm::radians(roll);
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

	void BaseCamera::CalcFrustumPlanes()
	{
		glm::vec3 cN = _position + _look * _zNear;
		glm::vec3 cF = _position + _look * _zFar;

		float hNear = 2.0f * tan(_fov / 2.0f) * _zNear;
		float wNear = hNear * _aspectRatio;
		float hFar = 2.0f * tan(_fov / 2.0f) * _zFar;
		float wFar = hFar * _aspectRatio;

		float hHNear = hNear / 2.0f;
		float hWNear = wNear / 2.0f;
		float hHFar = hFar / 2.0f;
		float hWFar = wFar / 2.0f;

		FarPts[0] = cF + _up * hHFar - _right * hWFar;
		FarPts[1] = cF - _up * hHFar - _right * hWFar;
		FarPts[2] = cF - _up * hHFar + _right * hWFar;
		FarPts[3] = cF + _up * hHFar + _right * hWFar;

		NearPts[0] = cN + _up * hHNear - _right * hWNear;
		NearPts[1] = cN - _up * hHNear - _right * hWNear;
		NearPts[2] = cN - _up * hHNear + _right * hWNear;
		NearPts[3] = cN + _up * hHNear + _right * hWNear;

		_clipingPlanes[0] = VPlane::FromPoints(NearPts[3], NearPts[0], FarPts[0]);
		_clipingPlanes[1] = VPlane::FromPoints(NearPts[1], NearPts[2], FarPts[2]);
		_clipingPlanes[2] = VPlane::FromPoints(NearPts[0], NearPts[1], FarPts[1]);
		_clipingPlanes[3] = VPlane::FromPoints(NearPts[2], NearPts[3], FarPts[2]);
		_clipingPlanes[4] = VPlane::FromPoints(NearPts[0], NearPts[3], NearPts[2]);
		_clipingPlanes[5] = VPlane::FromPoints(FarPts[3], FarPts[0], FarPts[1]);
	}

	bool BaseCamera::IsPointInFrustum(const glm::vec3 & point)
	{
		for (int i = 0; i < 6; i++)
			if (_clipingPlanes[i].GetDistance(point) < 0)
				return false;
		return true;
	}

	bool BaseCamera::IsSphereInFrustum(const glm::vec3 & center, const float radius)
	{
		for (int i = 0; i < 6; i++)
		{
			float d = _clipingPlanes[i].GetDistance(center);
			if (d < -radius)
				return false;
		}
		return true;
	}

	bool BaseCamera::IsBoxInFrustum(const glm::vec3 & min, const glm::vec3 & max)
	{
		glm::vec3 p;
		//glm::vec3 n;
		glm::vec3 N;
		for (int i = 0; i < 6; i++)
		{
			p = min;
			//n = Max;
			N = _clipingPlanes[i].Normal;
			if (N.x > 0)
			{
				p.x = max.x;
				//n.x = Min.x;
			}
			if (N.y > 0)
			{
				p.y = max.y;
				//n.y = Min.y;
			}
			if (N.z > 0)
			{
				p.z = max.z;
				//n.z = Min.z;
			}
			if (_clipingPlanes[i].GetDistance(p))
				return false;
		}
		return true;
	}

	void BaseCamera::GetFrustumPlanes(glm::vec4 planes[6])
	{
		for (int i = 0; i < 6; i++)
			planes[i] = glm::vec4(_clipingPlanes[i].Normal, _clipingPlanes[i].D);
	}



	FreeCamera::FreeCamera()
	{
		_translation = glm::vec3(0);
		_speed = 0.1f;
	}

	FreeCamera::~FreeCamera()
	{
	}

	void FreeCamera::Update()
	{
		glm::mat4 R = glm::yawPitchRoll(_yaw, _pitch, _roll);
		_position += _translation;

		_look = glm::vec3(R * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
		_up = glm::vec3(R * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
		_right = glm::cross(_look, _up);

		glm::vec3 tgt = _position + _look;
		_view = glm::lookAt(_position, tgt, _up);
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