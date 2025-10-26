#include "GeometryUtils.hpp"
#include "Config.hpp"

namespace GeometryUtils
{
	Array<Vec3> GenerateCylinderGridPositions(double radius, double height, int32 u_div, int32 v_div, double margin)
	{
		Array<Vec3> positions;
		if (u_div == 0 || v_div == 0)
		{
			return positions;
		}

		const double effectiveHeight = height - (margin * 2.0);
		if (effectiveHeight <= 0)
		{
			return positions;
		}

		for (int32 v = 0; v < v_div; ++v)
		{
			const double z = (static_cast<double>(v) / (v_div - 1) - 0.5) * effectiveHeight;
			for (int32 u = 0; u < u_div; ++u)
			{
				const double angle = (static_cast<double>(u) / u_div) * Math::TwoPi;
				const double x = radius * Math::Cos(angle);
				const double y = radius * Math::Sin(angle);
				positions.push_back({ x, y, z });
			}
		}
		return positions;
	}

	Optional<Vec3> GetLinePlaneIntersection(const Vec3& lineStart, const Vec3& lineEnd, double planeX)
	{
		const Vec3 direction = lineEnd - lineStart;

		// 線がx軸に平行な場合は交差しない
		if (Math::Abs(direction.x) < 1e-6)
		{
			return none;
		}

		// t = (planeX - lineStart.x) / direction.x
		const double t = (planeX - lineStart.x) / direction.x;

		// 交点を計算
		return lineStart + direction * t;
	}

	double CalculatePointToLineDistance(const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd)
	{
		const Vec3 lineVec = lineEnd - lineStart;
		const Vec3 pointVec = point - lineStart;

		const double lineLength = lineVec.length();
		if (lineLength < 1e-6)
		{
			return point.distanceFrom(lineStart);
		}

		const Vec3 lineDir = lineVec.normalized();
		const Vec3 cross = pointVec.cross(lineDir);

		return cross.length();
	}

	Optional<Vec3> GetRayPlaneIntersection(const Ray& ray, double planeX)
	{
		const Vec3 origin = ray.getOrigin();
		const Vec3 direction = ray.getDirection();

		// レイの方向がx軸に平行な場合は交差しない
		if (Math::Abs(direction.x) < 1e-6)
		{
			return none;
		}

		// t = (planeX - origin.x) / direction.x
		const double t = (planeX - origin.x) / direction.x;

		// 交点がレイの後方にある場合は無効
		if (t < 0)
		{
			return none;
		}

		// 交点を計算
		return origin + direction * t;
	}

	Vec3 GetMouseWorldPosition(const Vec2& mousePos, const DebugCamera3D& camera, double distance, bool constrainToPlane)
	{
		const Ray ray = camera.screenToRay(mousePos);

		if (constrainToPlane)
		{
			// x=3の平面との交点を計算
			const auto intersection = GetRayPlaneIntersection(ray, Config::DragPlaneX);
			if (intersection)
			{
				return *intersection;
			}
		}

		// デフォルトの距離ベースの計算
		const Vec3 origin = ray.getOrigin();
		const Vec3 direction = ray.getDirection();
		return origin + direction * distance;
	}
}