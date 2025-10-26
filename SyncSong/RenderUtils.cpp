#include "RenderUtils.hpp"
#include "Config.hpp"

namespace RenderUtils
{
	Texture CreateGradientTexture(const ColorF& topColor, const ColorF& bottomColor, int32 height)
	{
		Image gradientImage{ 1, static_cast<size_t>(height) };

		for (int32 y = 0; y < height; ++y)
		{
			const double t = y / static_cast<double>(height - 1);
			gradientImage[0][static_cast<size_t>(height - 1 - y)] = Color{ topColor.lerp(bottomColor, t) };
		}

		return Texture{ gradientImage };
	}

	Mesh CreateCylinderMesh(double radius, double height, int32 quality)
	{
		MeshData cylinderMesh = MeshData::Cylinder(radius, height, quality);
		cylinderMesh.rotate(Quaternion::RotateX(90_deg));
		return Mesh{ cylinderMesh };
	}

	void Setup3DScene()
	{
		Graphics3D::SetGlobalAmbientColor(ColorF{ 0.4 });
		Graphics3D::SetSunDirection(Vec3{ -1, -1, 0.5 }.normalized());
	}

	void Render3DScene(
		const MSRenderTexture& renderTexture,
		DebugCamera3D& camera,
		const Mesh& cylinderMesh,
		const Texture& gradientTexture,
		const Array<SphereState>& spheres,
		double rotationAngle,
		const DragState& dragState)
	{
		const ScopedRenderTarget3D target{ renderTexture.clear(Scene::GetBackground()) };
		Graphics3D::SetCameraTransform(camera);
		Setup3DScene();

		const auto transform = Mat4x4::RotateZ(rotationAngle);
		cylinderMesh.draw(transform, gradientTexture);

		// 球を描画
		for (int32 i = 0; i < spheres.size(); ++i)
		{
			const auto& sphere = spheres[i];
			ColorF color = sphere.isYellow ? Palette::Yellow : Palette::Gray;

			// ドラッグ中の球は少し透明にする
			if (dragState.isDragging && dragState.draggedSphereIndex == i)
			{
				color.a = 0.7;
			}

			 // スナップ候補の球をハイライト表示
			if (dragState.isDragging && dragState.snapCandidates.contains(i))
			{
				color = ColorF{ 0.0, 1.0, 0.5, 0.8 }; // 緑色でハイライト
			}

			// 取り付けられている球は回転変換を適用、取り外された球はそのまま描画
			if (sphere.isAttached)
			{
				Sphere{ sphere.position, Config::SphereRadius }.draw(transform, color);
			}
			else
			{
				// 取り外された球は回転変換を適用しない
				Sphere{ sphere.position, Config::SphereRadius }.draw(color);
			}
		}

		// デバッグ用：ドラッグ中はプレイヤーとドラッグ球を結ぶ線を描画
		if (dragState.isDragging && dragState.draggedSphereIndex >= 0)
		{
			const Vec3 playerPos = camera.getEyePosition();
			const Vec3 draggedPos = spheres[dragState.draggedSphereIndex].position;
			Line3D{ playerPos, draggedPos }.draw(ColorF{ 1.0, 0.0, 0.0, 0.5 });
		}
	}

	void RenderToScreen(const MSRenderTexture& renderTexture)
	{
		Graphics3D::Flush();
		renderTexture.resolve();
		Shader::LinearToScreen(renderTexture);
	}
}