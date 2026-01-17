#pragma once

#include "Math.h"


namespace UniDx {

/**
 * @brief 3x4 の列ベクトル（行列×ベクトル）行優先（最初の行から順番）行列
 */
struct alignas(16) BoneMat3x4
{
	Vector4 c0;
	Vector4 c1;
	Vector4 c2;

	static const BoneMat3x4 identity;

	constexpr BoneMat3x4() noexcept
		: c0(1, 0, 0, 0)
		, c1(0, 1, 0, 0)
		, c2(0, 0, 1, 0)
	{}

	constexpr BoneMat3x4(const Vector4& ic0, const Vector4& ic1, const Vector4& ic2) noexcept
		: c0(ic0), c1(ic1), c2(ic2)
	{}

	/// @brief Matrix4x4（row-major）から作る
	static constexpr BoneMat3x4 FromMatrix4x4(const Matrix4x4& m) noexcept
	{
		return BoneMat3x4(
			Vector4(m.m00, m.m10, m.m20, m.m30),
			Vector4(m.m01, m.m11, m.m21, m.m31),
			Vector4(m.m02, m.m12, m.m22, m.m32)
		);
	}

	/// @brief Matrix4x4 に戻す
	constexpr Matrix4x4 ToMatrix4x4() const noexcept
	{
		return Matrix4x4(
			c0.x, c1.x, c2.x, 0.0f,
			c0.y, c1.y, c2.y, 0.0f,
			c0.z, c1.z, c2.z, 0.0f,
			c0.w, c1.w, c2.w, 1.0f
		);
	}

	/// @brief 平行移動成分
	constexpr Vector3 translation() const noexcept { return Vector3(c0.w, c1.w, c2.w); }
	constexpr void setTranslation(const Vector3& t) noexcept { c0.w = t.x; c1.w = t.y; c2.w = t.z; }

	/// @brief 点を変換（平行移動込み）
	constexpr Vector3 TransformPoint(const Vector3& p) const noexcept
	{
		return Vector3(
			p.x * c0.x + p.y * c1.x + p.z * c2.x + c0.w,
			p.x * c0.y + p.y * c1.y + p.z * c2.y + c1.w,
			p.x * c0.z + p.y * c1.z + p.z * c2.z + c2.w
		);
	}

	/// @brief 方向ベクトルを変換（平行移動なし）
	constexpr Vector3 TransformVector(const Vector3& v) const noexcept
	{
		return Vector3(
			v.x * c0.x + v.y * c1.x + v.z * c2.x,
			v.x * c0.y + v.y * c1.y + v.z * c2.y,
			v.x * c0.z + v.y * c1.z + v.z * c2.z
		);
	}
};


} // namespace UniDx
