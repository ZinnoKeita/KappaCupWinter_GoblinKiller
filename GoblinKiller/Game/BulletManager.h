#pragma once
class BulletManager:public IGameObject
{
public:
	void Init();
	void Spawn(const Vector3& start, const Vector3& end);
	void Update();
	void Render(RenderContext& rc);

private:
	static const int MAX_BULLET = 48;//同時に飛べる弾の最大数

	//弾一発分のデータ
	struct BulletData {
		bool active = false;
		Vector3 pos = Vector3::Zero;
		Vector3 dir = Vector3::AxisZ;
		float remain = 0.0f;
	};

	ModelRender m_models[MAX_BULLET];
	BulletData m_bullets[MAX_BULLET];
};

