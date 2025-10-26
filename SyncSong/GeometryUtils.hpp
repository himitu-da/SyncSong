#pragma once
#include <Siv3D.hpp>

namespace GeometryUtils
{
	// �~���̋Ȗʏ�ɃO���b�h�𐶐�
	Array<Vec3> GenerateCylinderGridPositions(double radius, double height, int32 u_div, int32 v_div, double margin);

	// 3D���ƕ��� x=planeX �̌�_���v�Z
	Optional<Vec3> GetLinePlaneIntersection(const Vec3& lineStart, const Vec3& lineEnd, double planeX);

	// �_�ƒ����̋������v�Z�i���������Łj
	double CalculatePointToLineDistance(const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd);

	// x=planeX �̕��ʂƃ��C�̌�_���v�Z
	Optional<Vec3> GetRayPlaneIntersection(const Ray& ray, double planeX);

	// �}�E�X�ʒu�ɑΉ�����3D�ʒu���擾�i�J�������C���g�p�j
	Vec3 GetMouseWorldPosition(const Vec2& mousePos, const DebugCamera3D& camera, double distance = 5.0, bool constrainToPlane = false);
}