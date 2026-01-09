#pragma once

#include <cmath>
#include <DirectXMath.h>

namespace UniDx
{
	/**
	 * @file Math.h
	 * @brief 基本的な数学型 (ベクトル・クォータニオン・行列・色) とユーティリティ関数
	 */

	/** @brief 円周率 (float) */
	constexpr float PI = 3.141592654f;
	/** @brief 度 -> ラジアン変換係数 */
	constexpr float Deg2Rad = PI / 180.0f;
	/** @brief ラジアン -> 度変換係数 */
	constexpr float Rad2Deg = 180.0f / PI;

	/** @brief 2D ベクトル */
	struct Vector2 : DirectX::XMFLOAT2
	{
		constexpr Vector2() noexcept : XMFLOAT2(0.f, 0.f) {}
		constexpr Vector2(float ix, float iy) noexcept : XMFLOAT2(ix, iy) {}
		constexpr explicit Vector2(float ix) noexcept : XMFLOAT2(ix, ix) {}
		Vector2(const DirectX::XMFLOAT2& V) noexcept : XMFLOAT2(V.x, V.y) {}
		explicit Vector2(const DirectX::XMVECTOR& v) { XMStoreFloat2(this, v); }
		/** @brief 二乗長さを取得 */
		float sqrMagnitude() const noexcept { return x * x + y * y; }

		// 単項演算子
		constexpr Vector2 operator+() const noexcept { return *this; }
		constexpr Vector2 operator-() const noexcept { return Vector2(-x, -y); }

		static const Vector2 zero;
		static const Vector2 one;
		static const Vector2 up;
		static const Vector2 down;
		static const Vector2 right;
		static const Vector2 left;
		static const Vector2 positiveInfinity;
		static const Vector2 negativeInfinity;
	};
	[[nodiscard]] inline Vector2 Min(const Vector2& v1, const Vector2& v2) noexcept
	{
		return Vector2(v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y);
	}
	[[nodiscard]] inline Vector2 Max(const Vector2& v1, const Vector2& v2) noexcept
	{
		return Vector2(v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y);
	}

	/** @brief 3D ベクトル */
	struct Vector3 : DirectX::XMFLOAT3
	{
		constexpr Vector3() noexcept : XMFLOAT3(0.f, 0.f,0.f) {}
		constexpr Vector3(float ix, float iy, float iz) noexcept : XMFLOAT3(ix, iy, iz) {}
		constexpr explicit Vector3(float ix) noexcept : XMFLOAT3(ix, ix, ix) {}
		Vector3(const DirectX::XMFLOAT3& V) noexcept : XMFLOAT3(V.x, V.y, V.z) {}
		explicit Vector3(const DirectX::XMVECTOR& v) { XMStoreFloat3(this, v); }
		/** @brief 長さを取得 */
		float magnitude() const noexcept { return std::sqrt((x * x) + (y * y) + (z * z)); }
		/** @brief 二乗長さを取得 */
		float sqrMagnitude() const noexcept { return x * x + y * y + z * z; }
		/** @brief 正規化したベクトルを返す */
		[[nodiscard]] Vector3 normalized() const noexcept
		{
			const DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(this);
			const DirectX::XMVECTOR X = DirectX::XMVector3Normalize(v1);
			return Vector3(X);
		}

		// 単項演算子
		constexpr Vector3 operator+() const noexcept { return *this; }
		constexpr Vector3 operator-() const noexcept { return Vector3(-x, -y, -z); }

		static const Vector3 zero;
		static const Vector3 one;
		static const Vector3 up;
		static const Vector3 down;
		static const Vector3 right;
		static const Vector3 left;
		static const Vector3 forward;
		static const Vector3 back;
		static const Vector3 positiveInfinity;
		static const Vector3 negativeInfinity;
	};
	/** @brief ベクトルの内積 */
	[[nodiscard]] inline constexpr float Dot(const Vector3& v1, const Vector3& v2) noexcept { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
	/** @brief ベクトルの外積 */
	[[nodiscard]] inline Vector3 Cross(const Vector3& v1, const Vector3& v2) noexcept
	{
		using namespace DirectX;
		const XMVECTOR x1 = XMLoadFloat3(&v1);
		const XMVECTOR x2 = XMLoadFloat3(&v2);
		const XMVECTOR X = XMVector3Cross(x1, x2);
		return Vector3(X);
	}
	[[nodiscard]] inline Vector3 Min(const Vector3& v1, const Vector3& v2) noexcept
	{
		return Vector3(v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y, v1.z < v2.z ? v1.z : v2.z);
	}
	[[nodiscard]] inline Vector3 Max(const Vector3& v1, const Vector3& v2) noexcept
	{
		return Vector3(v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y, v1.z > v2.z ? v1.z : v2.z);
	}
	/** @brief 2点間の距離 */
	inline float Distance(const Vector3& v1, const Vector3& v2) noexcept
	{
		using namespace DirectX;
		const XMVECTOR x1 = XMLoadFloat3(&v1);
		const XMVECTOR x2 = XMLoadFloat3(&v2);
		const XMVECTOR V = XMVectorSubtract(x2, x1);
		const XMVECTOR X = XMVector3Length(V);
		return XMVectorGetX(X);
	}
	/** @brief 2点間の二乗距離 */
	inline float SqrDistance(const Vector3& v1, const Vector3& v2) noexcept
	{
		using namespace DirectX;
		const XMVECTOR x1 = XMLoadFloat3(&v1);
		const XMVECTOR x2 = XMLoadFloat3(&v2);
		const XMVECTOR V = XMVectorSubtract(x2, x1);
		const XMVECTOR X = XMVector3LengthSq(V);
		return XMVectorGetX(X);
	}

	/** @brief 4D ベクトル */
	struct Vector4 : DirectX::XMFLOAT4
	{
		constexpr Vector4() noexcept : XMFLOAT4(0.f, 0.f, 0.f, 0.f) {}
		constexpr Vector4(float ix, float iy, float iz, float iw) noexcept : XMFLOAT4(ix, iy, iz, iw) {}
		constexpr explicit Vector4(float ix) noexcept : XMFLOAT4(ix, ix, ix, ix) {}
		Vector4(const DirectX::XMFLOAT4& V) noexcept : XMFLOAT4(V.x, V.y, V.z, V.w) {}
		explicit Vector4(const DirectX::XMVECTOR& v) { XMStoreFloat4(this, v); }

		static const Vector4 zero;
		static const Vector4 one;

		// 単項演算子
		constexpr Vector4 operator+() const noexcept { return *this; }
		constexpr Vector4 operator-() const noexcept { return Vector4(-x, -y, -z, -w); }
	};
	[[nodiscard]] inline Vector4 Min(const Vector4& v1, const Vector4& v2) noexcept
	{
		return Vector4(v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y, v1.z < v2.z ? v1.z : v2.z, v1.w < v2.w ? v1.w : v2.w);
	}
	[[nodiscard]] inline Vector4 Max(const Vector4& v1, const Vector4& v2) noexcept
	{
		return Vector4(v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y, v1.z > v2.z ? v1.z : v2.z, v1.w > v2.w ? v1.w : v2.w);
	}

	/** @brief クォータニオン (回転表現) */
	struct Quaternion
	{
		float x, y, z, w;

		constexpr Quaternion() noexcept : x(0.f), y(0.f), z(0.f), w(1.f) {}
		constexpr Quaternion(float ix, float iy, float iz, float iw) noexcept : x(ix), y(iy), z(iz), w(iw) {}
		Quaternion(const DirectX::XMFLOAT4& V) noexcept : x(V.x), y(V.y), z(V.z), w(V.w) {}
		explicit Quaternion(const DirectX::XMVECTOR& v) { DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), v); }

		const DirectX::XMVECTOR XMLoad() const
		{
			return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this));
		}
		void XMStore(const DirectX::XMVECTOR& v)
		{
			XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), v);
		}

		static const Quaternion identity;

		/** @brief 指定軸・角度からクォータニオンを生成（度指定） */
		static Quaternion AngleAxis(float degree, Vector3 axis)
		{
			auto a = DirectX::XMLoadFloat3(&axis);
			return Quaternion(DirectX::XMQuaternionRotationAxis(a, degree * Deg2Rad));
		}
		/** @brief オイラー角（度）からクォータニオンを生成（Z->X->Y の順で合成） */
		static Quaternion Euler(float pitchDegree, float yawDegree, float rollDegree) noexcept
		{
			// pitch = X, yaw = Y, roll = Z
			// “かけ順” は Unity ライクに Z->X->Y（ZXY）で合成する
			using namespace DirectX;
			const float pitch = pitchDegree * Deg2Rad; // X
			const float yaw = yawDegree * Deg2Rad; // Y
			const float roll = rollDegree * Deg2Rad; // Z
			const XMVECTOR qz = XMQuaternionRotationAxis(g_XMIdentityR2, roll);
			const XMVECTOR qx = XMQuaternionRotationAxis(g_XMIdentityR0, pitch);
			const XMVECTOR qy = XMQuaternionRotationAxis(g_XMIdentityR1, yaw);

			const XMVECTOR q = XMQuaternionMultiply(XMQuaternionMultiply(qz, qx), qy);
			return Quaternion(q);
		}
		/** @brief fromDirからtoDirへ回転するクォータニオンを生成 */
		static Quaternion FromToRotation(const Vector3& fromDir, const Vector3& toDir) noexcept
		{
			using namespace DirectX;
			const XMVECTOR F = XMVector3Normalize(XMLoadFloat3(&fromDir));
			const XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&toDir));
			const float dot = XMVectorGetX(XMVector3Dot(F, T));
			if (dot >= 1.f)
			{
				return identity;
			}
			else if (dot <= -1.f)
			{
				XMVECTOR axis = XMVector3Cross(F, XMLoadFloat3(&Vector3::right));
				if (XMVector3NearEqual(XMVector3LengthSq(axis), g_XMZero, g_XMEpsilon))
				{
					axis = XMVector3Cross(F, XMLoadFloat3(&Vector3::up));
				}

				const XMVECTOR Q = XMQuaternionRotationAxis(axis, XM_PI);
				return Quaternion(Q);
			}
			else
			{
				const XMVECTOR C = XMVector3Cross(F, T);
				XMFLOAT4 result;
				XMStoreFloat4(&result, C);
				const float s = sqrtf((1.f + dot) * 2.f);
				result.x /= s;
				result.y /= s;
				result.z /= s;
				result.w = s * 0.5f;
				return Quaternion(result);
			}
		}
		/** @brief 前方向と上方向に回転するクォータニオンを生成 */
		static Quaternion LookRotation(const Vector3& forward, const Vector3& up) noexcept
		{
			using namespace DirectX;
			Quaternion q1 = FromToRotation(Vector3::forward, forward);
			Vector3 c = Cross(forward, up.normalized());
			const XMVECTOR C = XMLoadFloat3(&c);
			if (XMVector3NearEqual(XMVector3LengthSq(C), g_XMZero, g_XMEpsilon))
			{
				return q1;
			}
			const XMVECTOR U = XMVector3Rotate(XMLoadFloat3(&Vector3::up), q1.XMLoad());
			Quaternion q2 = FromToRotation(Vector3(U), up);
			return Quaternion(XMQuaternionMultiply(q2.XMLoad(), q1.XMLoad()));
		}

		// 単項演算子
		constexpr Quaternion operator+() const noexcept { return *this; }
		constexpr Quaternion operator-() const noexcept { return Quaternion{ -x, -y, -z, -w }; }

		explicit operator DirectX::XMFLOAT4& () { return *reinterpret_cast<DirectX::XMFLOAT4*>(this); }
		explicit operator const DirectX::XMFLOAT4& () const { return *reinterpret_cast<const DirectX::XMFLOAT4*>(this); }
	};
	/** @brief クォータニオンの逆 */
	[[nodiscard]] inline Quaternion Inverse(const Quaternion& from) noexcept
	{
		auto q = DirectX::XMLoadFloat4(&static_cast<const DirectX::XMFLOAT4&>(from));
		return Quaternion(DirectX::XMQuaternionInverse(q) );
	}

	/** @brief 色 (RGBA, float) */
	struct Color
	{
		float r, g, b, a;

		constexpr Color() noexcept : Color(0.f, 0.f, 0.f, 1.f) {}
		constexpr explicit Color(float ir, float ig, float ib, float ia = 1.0f) noexcept : r(ir), g(ig), b(ib), a(ia) {}

		const DirectX::XMVECTOR XMLoad() const
		{
			return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this));
		}
		void XMStore(const DirectX::XMVECTOR& v)
		{
			XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), v);
		}

		static const Color black;
		static const Color blue;
		static const Color cyan;
		static const Color green;
		static const Color magenta;
		static const Color red;
		static const Color white;
		static const Color yellow;

		// 単項演算子
		Color operator+() const noexcept { return *this; }
		Color operator-() const noexcept { return Color{ -r, -g, -b, -a }; }

		explicit operator DirectX::XMFLOAT4() const { return DirectX::XMFLOAT4(r, g, b, a); }
	};

	/** @brief 4x4 行列 (行優先) */
	struct Matrix4x4
	{
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;

		static const Matrix4x4 identity;

		Matrix4x4() : m00(0.f), m01(0.f), m02(0.f), m03(0.f),
					  m10(0.f), m11(0.f), m12(0.f), m13(0.f),
					  m20(0.f), m21(0.f), m22(0.f), m23(0.f),
					  m30(0.f), m31(0.f), m32(0.f), m33(0.f) { }
		constexpr Matrix4x4(
			float im00, float im01, float im02, float im03,
			float im10, float im11, float im12, float im13,
			float im20, float im21, float im22, float im23,
			float im30, float im31, float im32, float im33) noexcept
			: m00(im00), m01(im01), m02(im02), m03(im03),
			m10(im10), m11(im11), m12(im12), m13(im13),
			m20(im20), m21(im21), m22(im22), m23(im23),
			m30(im30), m31(im31), m32(im32), m33(im33) {
		}
		Matrix4x4(const DirectX::XMFLOAT4X4& f) noexcept
			: m00(f._11), m01(f._12), m02(f._13), m03(f._14),
			m10(f._21), m11(f._22), m12(f._23), m13(f._24),
			m20(f._31), m21(f._32), m22(f._33), m23(f._34),
			m30(f._41), m31(f._42), m32(f._43), m33(f._44) {
		}
		explicit Matrix4x4(DirectX::CXMMATRIX M) noexcept { XMStore(M); }
		/** @brief 逆行列 */
		[[nodiscard]] Matrix4x4 inverse() const
		{
			using namespace DirectX;
			const XMMATRIX M = XMLoad();
			XMVECTOR det;
			return Matrix4x4(XMMatrixInverse(&det, M));
		}
		/** @brief 行列から平行移動成分を取得 */
		Vector3 translation() const noexcept { return Vector3(m30, m31, m32); }
		/** @brief 平行移動成分を設定 */
		void setTranslation(const Vector3& v) { m30 = v.x; m31 = v.y; m32 = v.z; }
		/** @brief 行列をスケール・回転・平行移動に分解する（成功なら true） */
		bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const noexcept
		{
			using namespace DirectX;
			XMVECTOR s, r, t;
			if (!XMMatrixDecompose(&s, &r, &t, XMLoad()))
				return false;

			XMStoreFloat3(&scale, s);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&rotation), r);
			XMStoreFloat3(&translation, t);
			return true;
		}
		/** @brief 行列に位置をかけて変換 */
		Vector3 MultiplyPoint(const DirectX::XMFLOAT3& v) const noexcept
		{
			using namespace DirectX;
			const XMVECTOR V = XMLoadFloat3(&v);
			const XMMATRIX M = XMLoad();
			const XMVECTOR R = XMVector3Transform(V, M);
			return Vector3(R);
		}
		/** @brief 行列に方向ベクトルをかけて変換（平行移動は無視） */
		Vector3 MultiplyVector(const Vector3& v) const noexcept
		{
			using namespace DirectX;
			const XMVECTOR v1 = XMLoadFloat3(&v);
			const XMMATRIX M = XMLoad();
			const XMVECTOR X = XMVector3TransformNormal(v1, M);
			return Vector3(X);
		}

		const DirectX::XMMATRIX XMLoad() const
		{
			return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(this) );
		}
		void XMStore(const DirectX::XMMATRIX& mtx)
		{
			XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(this), mtx);
		}

		/** @brief クォータニオンから回転行列を生成 */
		static Matrix4x4 Rotate(const Quaternion& rotation) noexcept
		{
			using namespace DirectX;
			const XMVECTOR quatv = rotation.XMLoad();
			return Matrix4x4(XMMatrixRotationQuaternion(quatv));
		}
		/** @brief 平行移動行列を生成 */
		static Matrix4x4 Translate(const Vector3& position) noexcept
		{
			return Matrix4x4(DirectX::XMMatrixTranslation(position.x, position.y, position.z));
		}

		// 単項演算子
		Matrix4x4 operator+() const noexcept { return *this; }
		Matrix4x4 operator-() const noexcept
		{
			return Matrix4x4{
				-m00,-m01,-m02,-m03,
				-m10,-m11,-m12,-m13,
				-m20,-m21,-m22,-m23,
				-m30,-m31,-m32,-m33
			};
		}

		explicit operator DirectX::XMFLOAT4X4& ()
		{
			return *reinterpret_cast<DirectX::XMFLOAT4X4*>(this);
		}
		explicit operator const DirectX::XMFLOAT4X4&() const
		{
			return *reinterpret_cast<const DirectX::XMFLOAT4X4*>(this);
		}
	};

	// --- Vector3 operators (element-wise + scalar) ---
	inline constexpr Vector3 operator+(const Vector3& a, const Vector3& b) noexcept { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
	inline constexpr Vector3 operator-(const Vector3& a, const Vector3& b) noexcept { return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
	inline constexpr Vector3 operator*(const Vector3& a, const Vector3& b) noexcept { return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); }
	inline constexpr Vector3 operator/(const Vector3& a, const Vector3& b) noexcept { return Vector3(a.x / b.x, a.y / b.y, a.z / b.z); }

	inline constexpr Vector3 operator*(const Vector3& v, float s) noexcept { return Vector3(v.x * s, v.y * s, v.z * s); }
	inline constexpr Vector3 operator*(float s, const Vector3& v) noexcept { return v * s; }
	inline constexpr Vector3 operator/(const Vector3& v, float s) noexcept { return Vector3(v.x / s, v.y / s, v.z / s); }

	inline Vector3& operator+=(Vector3& a, const Vector3& b) noexcept { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
	inline Vector3& operator-=(Vector3& a, const Vector3& b) noexcept { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
	inline Vector3& operator*=(Vector3& a, const Vector3& b) noexcept { a.x *= b.x; a.y *= b.y; a.z *= b.z; return a; }
	inline Vector3& operator/=(Vector3& a, const Vector3& b) noexcept { a.x /= b.x; a.y /= b.y; a.z /= b.z; return a; }

	inline Vector3& operator*=(Vector3& v, float s) noexcept { v.x *= s; v.y *= s; v.z *= s; return v; }
	inline Vector3& operator/=(Vector3& v, float s) noexcept { v.x /= s; v.y /= s; v.z /= s; return v; }

	inline constexpr bool operator==(const Vector3& a, const Vector3& b) noexcept { return a.x == b.x && a.y == b.y && a.z == b.z; }
	inline constexpr bool operator!=(const Vector3& a, const Vector3& b) noexcept { return !(a == b); }

	// --- Vector2 operators ---
	inline constexpr Vector2 operator+(const Vector2& a, const Vector2& b) noexcept { return Vector2(a.x + b.x, a.y + b.y); }
	inline constexpr Vector2 operator-(const Vector2& a, const Vector2& b) noexcept { return Vector2(a.x - b.x, a.y - b.y); }
	inline constexpr Vector2 operator*(const Vector2& a, const Vector2& b) noexcept { return Vector2(a.x * b.x, a.y * b.y); }
	inline constexpr Vector2 operator/(const Vector2& a, const Vector2& b) noexcept { return Vector2(a.x / b.x, a.y / b.y); }

	inline constexpr Vector2 operator*(const Vector2& v, float s) noexcept { return Vector2(v.x * s, v.y * s); }
	inline constexpr Vector2 operator*(float s, const Vector2& v) noexcept { return v * s; }
	inline constexpr Vector2 operator/(const Vector2& v, float s) noexcept { return Vector2(v.x / s, v.y / s); }

	inline Vector2& operator+=(Vector2& a, const Vector2& b) noexcept { a.x += b.x; a.y += b.y; return a; }
	inline Vector2& operator-=(Vector2& a, const Vector2& b) noexcept { a.x -= b.x; a.y -= b.y; return a; }
	inline Vector2& operator*=(Vector2& a, const Vector2& b) noexcept { a.x *= b.x; a.y *= b.y; return a; }
	inline Vector2& operator/=(Vector2& a, const Vector2& b) noexcept { a.x /= b.x; a.y /= b.y; return a; }

	inline Vector2& operator*=(Vector2& v, float s) noexcept { v.x *= s; v.y *= s; return v; }
	inline Vector2& operator/=(Vector2& v, float s) noexcept { v.x /= s; v.y /= s; return v; }

	inline constexpr bool operator==(const Vector2& a, const Vector2& b) noexcept { return a.x == b.x && a.y == b.y; }
	inline constexpr bool operator!=(const Vector2& a, const Vector2& b) noexcept { return !(a == b); }

	// --- Quaternion operators ---
	inline constexpr Quaternion operator+(const Quaternion& a, const Quaternion& b) noexcept { return Quaternion{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
	inline constexpr Quaternion operator-(const Quaternion& a, const Quaternion& b) noexcept { return Quaternion{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
	inline constexpr Quaternion operator*(const Quaternion& q, float s) noexcept { return Quaternion{ q.x * s, q.y * s, q.z * s, q.w * s }; }
	inline constexpr Quaternion operator*(float s, const Quaternion& q) noexcept { return q * s; }
	inline constexpr Quaternion operator/(const Quaternion& q, float s) noexcept { return Quaternion{ q.x / s, q.y / s, q.z / s, q.w / s }; }

	inline Quaternion& operator+=(Quaternion& a, const Quaternion& b) noexcept { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
	inline Quaternion& operator-=(Quaternion& a, const Quaternion& b) noexcept { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
	inline Quaternion& operator*=(Quaternion& q, float s) noexcept { q.x *= s; q.y *= s; q.z *= s; q.w *= s; return q; }
	inline Quaternion& operator/=(Quaternion& q, float s) noexcept { q.x /= s; q.y /= s; q.z /= s; q.w /= s; return q; }

	inline bool operator==(const Quaternion& a, const Quaternion& b) noexcept { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
	inline bool operator!=(const Quaternion& a, const Quaternion& b) noexcept { return !(a == b); }

	inline Quaternion operator*(const Quaternion& a, const Quaternion& b) noexcept
	{
		using namespace DirectX;
		const XMVECTOR A = XMVectorSet(a.x, a.y, a.z, a.w);
		const XMVECTOR B = XMVectorSet(b.x, b.y, b.z, b.w);
		const XMVECTOR R = XMQuaternionMultiply(A, B);
		return Quaternion(R);
	}
	inline Vector3 operator*(const DirectX::XMFLOAT3& v, const Quaternion& q) noexcept
	{
		using namespace DirectX;
		const XMVECTOR Q = q.XMLoad();
		const XMVECTOR V = XMLoadFloat3(&v);
		const XMVECTOR R = XMVector3Rotate(V, Q);
		return Vector3(R);
	}

	// --- Color operators ---
	inline Color operator+(const Color& a, const Color& b) noexcept { return Color{ a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a }; }
	inline Color operator-(const Color& a, const Color& b) noexcept { return Color{ a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a }; }
	inline Color operator*(const Color& a, const Color& b) noexcept { return Color{ a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a }; }
	inline Color operator*(const Color& c, float s) noexcept { return Color{ c.r * s, c.g * s, c.b * s, c.a * s }; }
	inline Color operator*(float s, const Color& c) noexcept { return c * s; }
	inline Color operator/(const Color& c, float s) noexcept { return Color{ c.r / s, c.g / s, c.b / s, c.a / s }; }

	inline Color& operator+=(Color& a, const Color& b) noexcept { a.r += b.r; a.g += b.g; a.b += b.b; a.a += b.a; return a; }
	inline Color& operator-=(Color& a, const Color& b) noexcept { a.r -= b.r; a.g -= b.g; a.b -= b.b; a.a -= b.a; return a; }
	inline Color& operator*=(Color& a, float s) noexcept { a.r *= s; a.g *= s; a.b *= s; a.a *= s; return a; }

	inline bool operator==(const Color& a, const Color& b) noexcept { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
	inline bool operator!=(const Color& a, const Color& b) noexcept { return !(a == b); }

	// --- Matrix4x4 operators using DirectXMath for performance ---
	inline Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b) noexcept
	{
		using namespace DirectX;
		const XMMATRIX A = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&a));
		const XMMATRIX B = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&b));
		const XMMATRIX R = XMMatrixMultiply(A, B);
		return Matrix4x4(R);
	}

	inline Matrix4x4& operator*=(Matrix4x4& a, const Matrix4x4& b) noexcept { a = a * b; return a; }
	inline bool operator==(const Matrix4x4& a, const Matrix4x4& b) noexcept
	{
		return a.m00 == b.m00 && a.m01 == b.m01 && a.m02 == b.m02 && a.m03 == b.m03 &&
			a.m10 == b.m10 && a.m11 == b.m11 && a.m12 == b.m12 && a.m13 == b.m13 &&
			a.m20 == b.m20 && a.m21 == b.m21 && a.m22 == b.m22 && a.m23 == b.m23 &&
			a.m30 == b.m30 && a.m31 == b.m31 && a.m32 == b.m32 && a.m33 == b.m33;
	}
	inline bool operator!=(const Matrix4x4& a, const Matrix4x4& b) noexcept { return !(a == b); }

	inline Vector3 operator*(const DirectX::XMFLOAT3& v, const Matrix4x4& m) noexcept
	{ return m.MultiplyPoint(v); }

	/** @brief 各型の文字列化（ワイド文字列） */
	inline std::wstring ToString(const Vector2& v) { return ToString(DirectX::XMFLOAT2(v)); }
	inline std::wstring ToString(const Vector3& v) { return ToString(DirectX::XMFLOAT3(v)); }
	inline std::wstring ToString(const Vector4& v) { return ToString(DirectX::XMFLOAT4(v)); }
	inline std::wstring ToString(const Color& v) { return ToString(DirectX::XMFLOAT4(v)); }
	inline std::wstring ToString(const Quaternion& v) { return ToString(DirectX::XMFLOAT4(v)); }

} // namespace UniDx