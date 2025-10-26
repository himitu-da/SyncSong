#pragma once
#include <Siv3D.hpp>

namespace GeometryUtils
{
	// 円筒の曲面上にグリッドを生成
	Array<Vec3> GenerateCylinderGridPositions(double radius, double height, int32 u_div, int32 v_div, double margin);

	// 3D線と平面 x=planeX の交点を計算
	Optional<Vec3> GetLinePlaneIntersection(const Vec3& lineStart, const Vec3& lineEnd, double planeX);

	// 点と直線の距離を計算（無限直線版）
	double CalculatePointToLineDistance(const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd);

	// x=planeX の平面とレイの交点を計算
	Optional<Vec3> GetRayPlaneIntersection(const Ray& ray, double planeX);

	// マウス位置に対応する3D位置を取得（カメラレイを使用）
	Vec3 GetMouseWorldPosition(const Vec2& mousePos, const DebugCamera3D& camera, double distance = 5.0, bool constrainToPlane = false);
}