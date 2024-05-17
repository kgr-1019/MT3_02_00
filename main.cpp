#include <Novice.h>
#include"Matrix4x4.h"
#include"Vector3.h"
#include<cmath>
#include"assert.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include"imgui.h"
const char kWindowTitle[] = "LD2B_04_コマツザキ_カガリ_タイトル";


struct Sphere
{
	Vector3 center;// 中心点
	float radius;// 半径
};

// 直線
struct Line {
	Vector3 origin;// 始点
	Vector3 diff;// 終点への差分ベクトル
};

// 半直線
struct Ray {
	Vector3 origin;// 始点
	Vector3 diff;// 終点への差分ベクトル
};

// 線分
struct Segment {
	Vector3 origin;// 始点
	Vector3 diff;// 終点への差分ベクトル
};



// プロトタイプ宣言
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,uint32_t color);

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(const Vector3& rotate);
// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(Vector3 rotate);
// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(Vector3 rotate);
// XYZ合成
Matrix4x4 MultiplyXYZ(const Matrix4x4& rotateX, const Matrix4x4& rotateYZ);
// アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& S, const Vector3& R, const Vector3& T);
// 逆行列
Matrix4x4 Inverse(Matrix4x4 cameraMatrix);
// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
// ビューポート行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
// 座標変換
Vector3 Transform(const Vector3& point, const Matrix4x4& transformMatrix);
// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2);
// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);
// 乗算
Vector3 Multiply(float k, const Vector3& v1);
// 正規化
Vector3 Normalize(const Vector3& v2);
// 内積
float Dot(const Vector3& v1, const Vector3& v2);
// 正射影ベクトル
Vector3 Project(const Vector3& v1, const Vector3& v2);
// 最近接点
Vector3 ClosestPoint(const Vector3& point, const Segment& segment);



// 関数の定義

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[1][1] = std::cos(rotate);
	result.m[1][2] = std::sin(rotate);
	result.m[2][1] = -std::sin(rotate);
	result.m[2][2] = std::cos(rotate);
	result.m[0][0] = 1;
	result.m[3][3] = 1;

	return result;
}

// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate);
	result.m[0][2] = -std::sin(rotate);
	result.m[1][1] = 1;
	result.m[2][0] = std::sin(rotate);
	result.m[2][2] = std::cos(rotate);
	result.m[3][3] = 1;

	return result;
}

// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate);
	result.m[0][1] = std::sin(rotate);
	result.m[1][0] = -std::sin(rotate);
	result.m[1][1] = std::cos(rotate);
	result.m[2][2] = 1;
	result.m[3][3] = 1;

	return result;
}

// XYZ合成
Matrix4x4 MultiplyXYZ(const Matrix4x4& rotateX, const Matrix4x4& rotateYZ)
{
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += rotateX.m[i][k] * rotateYZ.m[k][j];
			}
		}
	}

	return result;
}

// アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& S, const Vector3& R, const Vector3& T)
{
	Matrix4x4 result{};

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(R.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(R.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(R.z);
	Matrix4x4 rotateXYZMatrix = MultiplyXYZ(rotateXMatrix, MultiplyXYZ(rotateYMatrix, rotateZMatrix));


	result.m[0][0] = S.x * rotateXYZMatrix.m[0][0];
	result.m[0][1] = S.x * rotateXYZMatrix.m[0][1];
	result.m[0][2] = S.x * rotateXYZMatrix.m[0][2];
	result.m[1][0] = S.y * rotateXYZMatrix.m[1][0];
	result.m[1][1] = S.y * rotateXYZMatrix.m[1][1];
	result.m[1][2] = S.y * rotateXYZMatrix.m[1][2];
	result.m[2][0] = S.z * rotateXYZMatrix.m[2][0];
	result.m[2][1] = S.z * rotateXYZMatrix.m[2][1];
	result.m[2][2] = S.z * rotateXYZMatrix.m[2][2];
	result.m[3][0] = T.x;
	result.m[3][1] = T.y;
	result.m[3][2] = T.z;
	result.m[3][3] = 1;

	return result;
}

// 逆行列
Matrix4x4 Inverse(Matrix4x4 cameraMatrix)
{
	Matrix4x4 result{};

	float abs;//絶対値はint型にする

	// |A|
	abs = (cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]
			);

	// 1/A
	result.m[0][0] = 1.0f / abs * (
		(cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[0][1] = 1.0f / abs * (
		-(cameraMatrix.m[0][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		+ cameraMatrix.m[0][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1] + cameraMatrix.m[0][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3] + cameraMatrix.m[0][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2]
		);
	result.m[0][2] = 1.0f / abs * (
		(cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[3][2])
		);
	result.m[0][3] = 1.0f / abs * (
		-(cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2])
		);

	result.m[1][0] = 1.0f / abs * (
		-(cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[1][1] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[1][2] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[3][2])
		);
	result.m[1][3] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2])
		);

	result.m[2][0] = 1.0f / abs * (
		(cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1])
		);
	result.m[2][1] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1])
		);
	result.m[2][2] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[3][1])
		);
	result.m[2][3] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1])
		);

	result.m[3][0] = 1.0f / abs * (
		-(cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2]) + (cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1])
		);
	result.m[3][1] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2]) + (cameraMatrix.m[0][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[0][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2]) - (cameraMatrix.m[0][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1])
		);
	result.m[3][2] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[3][2]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[3][2]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[3][1])
		);
	result.m[3][3] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1])
		- (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1])
		);


	return result;
}

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 result{};

	result.m[0][0] = (1 / aspectRatio) * 1 / std::tan(fovY / 2);
	result.m[1][1] = 1 / std::tan(fovY / 2);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);

	return result;
}

// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 result{};

	result.m[0][0] = width / 2;
	result.m[1][1] = -height / 2;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2);
	result.m[3][1] = top + (height / 2);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1;

	return result;
}

// 座標変換
Vector3 Transform(const Vector3& point, const Matrix4x4& transformMatrix)
{
	Vector3 result{};

	result.x = point.x * transformMatrix.m[0][0] + point.y * transformMatrix.m[1][0] + point.z * transformMatrix.m[2][0] + 1.0f * transformMatrix.m[3][0];
	result.y = point.x * transformMatrix.m[0][1] + point.y * transformMatrix.m[1][1] + point.z * transformMatrix.m[2][1] + 1.0f * transformMatrix.m[3][1];
	result.z = point.x * transformMatrix.m[0][2] + point.y * transformMatrix.m[1][2] + point.z * transformMatrix.m[2][2] + 1.0f * transformMatrix.m[3][2];
	float w = point.x * transformMatrix.m[0][3] + point.y * transformMatrix.m[1][3] + point.z * transformMatrix.m[2][3] + 1.0f * transformMatrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2)
{
	Vector3 add{};
	add.x = v1.x + v2.x;
	add.y = v1.y + v2.y;
	add.z = v1.z + v2.z;
	return add;
}

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2)
{
	Vector3 subtract{};
	subtract.x = v1.x - v2.x;
	subtract.y = v1.y - v2.y;
	subtract.z = v1.z - v2.z;
	return subtract;
}

// 乗算
Vector3 Multiply(float k, const Vector3& v1)
{
	Vector3 multiply{};
	multiply.x = k * v1.x;
	multiply.y = k * v1.y;
	multiply.z = k * v1.z;
	return multiply;
}

// 正規化(v / ||v||)
Vector3 Normalize(const Vector3& v2)
{
	Vector3 normalize{};
	normalize.x = v2.x / sqrtf(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	normalize.y = v2.y / sqrtf(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	normalize.z = v2.z / sqrtf(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	return normalize;
}

// 内積(v1・v2)
float Dot(const Vector3& v1, const Vector3& v2)
{
	float dot = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return dot;
}

// 正射影ベクトル
Vector3 Project(const Vector3& v1, const Vector3& v2)
{
	float c{};
	Vector3 normalize{};
	Vector3 result{};

	normalize = Normalize(v2);// b^
	c = Dot(v1, normalize);// a・b^
	result = Multiply(c, normalize);// (a・b^)b^

	return result;
};

// 最近接点
Vector3 ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 proj_a{};
	Vector3 cp{};

	proj_a = Project(Subtract(point, segment.origin), segment.diff);// (a・b^)b^
	cp = Add(segment.origin, proj_a);// o + proj_a

	return cp;
};


// Gridを表示する疑似コード
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix)
{
	const float kGridHalfWidth = 2.0f;// Gridの半分の幅
	const uint32_t kSubdivision = 10;// 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);// 1つ分の長さ

	// 奥から手前
	Vector3 startLineZ;
	Vector3 endLineZ;
	// 左から右
	Vector3 startLineX;
	Vector3 endLineX;

	// 奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex)
	{
		// 上の情報を使ってワールド座標系上の始点と終点を求める
		startLineZ = Vector3{ xIndex * kGridEvery - kGridHalfWidth, 0.0f, kGridHalfWidth };
		endLineZ = Vector3{ xIndex * kGridEvery - kGridHalfWidth, 0.0f , -kGridHalfWidth };

		// スクリーン座標系まで変換をかける
		// 奥の始点
		Matrix4x4 worldMatrixStart = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, startLineZ);// ワールド座標に変換
		Matrix4x4 worldViewProjectionMatrixStart = MultiplyXYZ(worldMatrixStart, viewProjectionMatrix);// WVPMatrixを作る
		Vector3 ndcVertexStart = Transform(Vector3{}, worldViewProjectionMatrixStart);// NDC(正規化デバイス座標系)
		Vector3 screenVerticesStart = Transform(ndcVertexStart, viewportMatrix);// スクリーン座標に変換
		// 手前の終点
		Matrix4x4 worldMatrixEnd = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, endLineZ);// ワールド座標行列の作成
		Matrix4x4 worldViewProjectionMatrixEnd = MultiplyXYZ(worldMatrixEnd, viewProjectionMatrix);// WVPMatrixを作る
		Vector3 ndcVertexEnd = Transform(Vector3{}, worldViewProjectionMatrixEnd);// NDC(正規化デバイス座標系)
		Vector3 screenVerticesEnd = Transform(ndcVertexEnd, viewportMatrix);// スクリーン座標に変換

		// 変換した座標を使って表示
		Novice::DrawLine((int)screenVerticesStart.x, (int)screenVerticesStart.y, (int)screenVerticesEnd.x, (int)screenVerticesEnd.y, 0xAAAAAAFF);
	}

	// 左から右
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex)
	{
		// 上の情報を使ってワールド座標系上の始点と終点を求める
		startLineX = Vector3{ kGridHalfWidth, 0.0f, zIndex * kGridEvery - kGridHalfWidth };
		endLineX = Vector3{ -kGridHalfWidth, 0.0f, zIndex * kGridEvery - kGridHalfWidth };

		// スクリーン座標系まで変換をかける
		// 左の始点
		Matrix4x4 worldMatrixLeft = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, startLineX);// ワールド座標行列の作成
		Matrix4x4 worldViewProjectionMatrixLeft = MultiplyXYZ(worldMatrixLeft, viewProjectionMatrix);// WVPMatrixを作る
		Vector3 ndcVertexLeft = Transform(Vector3{}, worldViewProjectionMatrixLeft);// NDC(正規化デバイス座標系)
		Vector3 screenVerticesLeft = Transform(ndcVertexLeft, viewportMatrix);// スクリーン座標に変換
		// 右の終点
		Matrix4x4 worldMatrixRight = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, endLineX);// ワールド座標行列の作成
		Matrix4x4 worldViewProjectionMatrixRight = MultiplyXYZ(worldMatrixRight, viewProjectionMatrix);// WVPMatrixを作る
		Vector3 ndcVertexRight = Transform(Vector3{}, worldViewProjectionMatrixRight);// NDC(正規化デバイス座標系)
		Vector3 screenVerticesRight = Transform(ndcVertexRight, viewportMatrix);// スクリーン座標に変換

		// 変換した座標を使って表示
		Novice::DrawLine((int)screenVerticesLeft.x, (int)screenVerticesLeft.y, (int)screenVerticesRight.x, (int)screenVerticesRight.y, 0xAAAAAAFF);
	}
}

// Sphereを表示する疑似コード
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,uint32_t color)
{
	float pi = float(M_PI);
	const uint32_t kSubdivision = 20;// 分割数
	const float kLonEvery = (2.0f * pi) / kSubdivision;// 経度分割1つ分の角度Φd
	const float kLatEvery = pi / kSubdivision;// 緯度分割1つ分の角度θd
	// world座標系でのa,b,cを求める
	Vector3 a, b, c;

	// 経度の方向に分割 -π/2 ～ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
	{
		float lat = -pi / 2.0f + kLatEvery * latIndex;// 現在の緯度θ

		// 経度の方向に分割 0 ～ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex)
		{
			float lon = lonIndex * kLonEvery;// 現在の経度Φ

			// ワールド座標系での頂点を求める
			a = {
				(sphere.radius) * std::cos(lat) * std::cos(lon) + sphere.center.x,
				(sphere.radius) * std::sin(lat) + sphere.center.y,
				(sphere.radius) * std::cos(lat) * std::sin(lon) + sphere.center.z
			};

			b = {
				(sphere.radius) * std::cos(lat + kLatEvery) * std::cos(lon) + sphere.center.x,
				(sphere.radius) * std::sin(lat + kLatEvery) + sphere.center.y,
				(sphere.radius) * std::cos(lat + kLatEvery) * std::sin(lon) + sphere.center.z
			};

			c = {
				(sphere.radius) * std::cos(lat) * std::cos(lon + kLonEvery) + sphere.center.x,
				(sphere.radius) * std::sin(lat) + sphere.center.y,
				(sphere.radius) * std::cos(lat) * std::sin(lon + kLonEvery) + sphere.center.z
			};

			// a,b,cをScreen座標系まで変換
			// スクリーン座標系まで変換をかける
			Matrix4x4 worldMatrixA = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, a);
			Matrix4x4 worldMatrixB = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, b);
			Matrix4x4 worldMatrixC = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, c);
			// WVPMatrix
			Matrix4x4 worldViewProjectionMatrixA = MultiplyXYZ(worldMatrixA, viewProjectionMatrix);
			Matrix4x4 worldViewProjectionMatrixB = MultiplyXYZ(worldMatrixB, viewProjectionMatrix);
			Matrix4x4 worldViewProjectionMatrixC = MultiplyXYZ(worldMatrixC, viewProjectionMatrix);
			// NDC(正規化デバイス座標系)
			Vector3 ndcVertexA = Transform(Vector3{}, worldViewProjectionMatrixA);
			Vector3 ndcVertexB = Transform(Vector3{}, worldViewProjectionMatrixB);
			Vector3 ndcVertexC = Transform(Vector3{}, worldViewProjectionMatrixC);
			// スクリーン座標へ変換
			Vector3 screenVerticesA = Transform(ndcVertexA, viewportMatrix);
			Vector3 screenVerticesB = Transform(ndcVertexB, viewportMatrix);
			Vector3 screenVerticesC = Transform(ndcVertexC, viewportMatrix);


			// ab,acで線を引く
			Novice::DrawLine((int)screenVerticesA.x, (int)screenVerticesA.y, (int)screenVerticesB.x, (int)screenVerticesB.y, color);
			Novice::DrawLine((int)screenVerticesA.x, (int)screenVerticesA.y, (int)screenVerticesC.x, (int)screenVerticesC.y, color);
		}
	}
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


	// 画面サイズ
	float kWindowsWidth = 1280.0f;
	float kWindowsHeight = 720.0f;

	// カメラ
	Vector3 cameraScale{ 1.0f,1.0f,1.0f };
	Vector3 cameraTranslate{ 0.0f,1.9f,-6.49f };// カメラの位置
	Vector3 cameraRotate{ 0.26f,0.0f,0.0f };// カメラの角度

	// スフィアの定義
	Sphere sphere{{0.0f,0.0f,0.0f},0.5f,};

	// 
	Segment segment{ {-2.0f,-1.0f,0.0f},{3.0f,2.0f,2.0f} };
	Vector3 point{ -1.5f,0.6f,0.6f };

	uint32_t color[3]{ WHITE,RED,BLACK };


	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///


		Matrix4x4 worldMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraTranslate);

		Matrix4x4 viewMatrix = Inverse(worldMatrix);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowsWidth) / float(kWindowsHeight), 1.0f, 0.0f);

		Matrix4x4 worldViewProjectionMatrix = MultiplyXYZ(viewMatrix, projectionMatrix);

		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowsWidth), float(kWindowsHeight), 0.0f, 1.0f);

		// 正射影ベクトル
		Vector3 project = Project(Subtract(point, segment.origin), segment.diff);
		// 最近接点
		Vector3 closestPoint = ClosestPoint(point, segment);

		Sphere pointSphere{ point,0.01f };
		Sphere closestPointSphere{ closestPoint,0.01f };

		// 線の始点終点
		Vector3 start = Transform(Transform(segment.origin, worldViewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), worldViewProjectionMatrix), viewportMatrix);

		// グリッド
		DrawGrid(worldViewProjectionMatrix, viewportMatrix);
		// 点
		DrawSphere(pointSphere, worldViewProjectionMatrix, viewportMatrix, color[1]);
		DrawSphere(closestPointSphere, worldViewProjectionMatrix, viewportMatrix, color[2]);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///


		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), color[0]);
		ImGui::Begin("Windou");
		ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::End();

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
