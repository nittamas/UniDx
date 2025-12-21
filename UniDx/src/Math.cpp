#include "pch.h"
#include <limits>

namespace UniDx{

	// Vector2
	const Vector2 Vector2::zero = Vector2(0.f, 0.f);
	const Vector2 Vector2::one = Vector2(1.f, 1.f);
	const Vector2 Vector2::up = Vector2(0.f, 1.f);
	const Vector2 Vector2::down = Vector2(0.f, -1.f);
	const Vector2 Vector2::right = Vector2(1.f, 0.f);
	const Vector2 Vector2::left = Vector2(-1.f, 0.f);
	const Vector2 Vector2::positiveInfinity = Vector2(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	const Vector2 Vector2::negativeInfinity = Vector2(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

	// Vector3
	const Vector3 Vector3::zero = Vector3(0.f, 0.f, 0.f);
	const Vector3 Vector3::one = Vector3(1.f, 1.f, 1.f);
	const Vector3 Vector3::up = Vector3(0.f, 1.f, 0.f);
	const Vector3 Vector3::down = Vector3(0.f, -1.f, 0.f);
	const Vector3 Vector3::right = Vector3(1.f, 0.f, 0.f);
	const Vector3 Vector3::left = Vector3(-1.f, 0.f, 0.f);
	const Vector3 Vector3::forward = Vector3(0.f, 0.f, 1.f);
	const Vector3 Vector3::back = Vector3(0.f, 0.f, -1.f);
	const Vector3 Vector3::positiveInfinity = Vector3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	const Vector3 Vector3::negativeInfinity = Vector3(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

	// Vector4
	const Vector4 Vector4::zero = Vector4(0.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::one = Vector4(1.f, 1.f, 1.f, 1.f);

	// Quaternion
	const Quaternion Quaternion::identity = Quaternion(0.f, 0.f, 0.f, 1.f);

	// Color
	const Color Color::black = Color{ 0.f, 0.f, 0.f, 1.f };
	const Color Color::blue = Color{ 0.f, 0.f, 1.f, 1.f };
	const Color Color::cyan = Color{ 0.f, 1.f, 1.f, 1.f };
	const Color Color::green = Color{ 0.f, 1.f, 0.f, 1.f };
	const Color Color::magenta = Color{ 1.f, 0.f, 1.f, 1.f };
	const Color Color::red = Color{ 1.f, 0.f, 0.f, 1.f };
	const Color Color::white = Color{ 1.f, 1.f, 1.f, 1.f };
	const Color Color::yellow = Color{ 1.f, 1.f, 0.f, 1.f };

	// Matrix4x4
	const Matrix4x4 Matrix4x4::identity = Matrix4x4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	);

}
