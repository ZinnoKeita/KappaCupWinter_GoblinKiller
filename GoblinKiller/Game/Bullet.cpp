#include "stdafx.h"
#include "Bullet.h"

namespace {
	const float BULLET_SPEED = 5000.0f;//弾の速さ（1秒当たり）
	const float BULLET_SCALE = 0.3;//弾の大きさ（モデルに合わせて調整）
}

void Bullet::Setup(const Vector3& start, const Vector3& end) {
	m_position = start;

	//発射位置から着弾点へ向かうベクトル
	Vector3 diff = end - start;
	m_remain = diff.Length();//着くまでの距離

	if (m_remain > 0.001f) {
		m_dir = diff;
		m_dir.Normalize();//長さ1にして、向きだけにする
	}
	else {
		m_remain = 0.0f;
	}
}

bool Bullet::Start() {
	m_modelRender.Init("Assets/modelData/Bullet.tkm");
	m_modelRender.SetShadowCasterFlag(false);
	m_modelRender.SetScale(BULLET_SCALE, BULLET_SCALE, BULLET_SCALE);
	m_modelRender.SetPosition(m_position);
	m_modelRender.Update();
	return true;
}

void Bullet::Update() {
	float dt = g_gameTime->GetFrameDeltaTime();
	float move = BULLET_SPEED * dt;//このフレームで進む距離

	if (move >= m_remain) {
		DeleteGO(this);
		return;
	}

	m_position += m_dir * move;//飛ぶ方向を決める
	m_remain -= move;

	m_modelRender.SetPosition(m_position);
	m_modelRender.Update();
}

void Bullet::Render(RenderContext& rc) {
	m_modelRender.Draw(rc);
}