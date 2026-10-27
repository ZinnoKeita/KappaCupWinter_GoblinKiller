#include "stdafx.h"
#include "BulletManager.h"

namespace {
	const float BULLET_SPEED = 5000.0f;//弾の速さ（1秒当たり）
	const float BULLET_SCALE = 0.3;//弾の大きさ（モデルに合わせて調整）
}

void BulletManager::Init() {
	for (int i = 0;i < MAX_BULLET;i++) {
		m_models[i].Init("Assets/modelData/Bullet.tkm");
		m_models[i].SetShadowCasterFlag(false);
		m_models[i].SetScale(BULLET_SCALE, BULLET_SCALE, BULLET_SCALE);
		m_bullets[i].active = false;
	}
}

void BulletManager::Spawn(const Vector3& start, const Vector3& end) {
	Vector3 diff = end - start;
	float length = diff.Length();
	if (length < 0.001f) {
		return;
	}

	//空いてる弾を探して使う
	for (int i = 0;i < MAX_BULLET; i++) {
		if (m_bullets[i].active) {
			continue;
		}
		m_bullets[i].active = true;
		m_bullets[i].pos = start;
		m_bullets[i].remain = length;
		m_bullets[i].dir = diff;
		m_bullets[i].dir.Normalize();
		return;
	}
	//全部使用中なら何もしない 見た目が出ないだけでダメージに影響なし
}

void BulletManager::Update() {
	float dt = g_gameTime->GetFrameDeltaTime();
	float move = BULLET_SPEED * dt;//このフレームで進む距離

	for (int i = 0;i < MAX_BULLET;i++) {
		if (!m_bullets[i].active) {
			continue;
		}

		if (move >= m_bullets[i].remain) {
			m_bullets[i].active = false;
			continue;
		}

		m_bullets[i].pos += m_bullets[i].dir * move;
		m_bullets[i].remain -= move;

		m_models[i].SetPosition(m_bullets[i].pos);
		m_models[i].Update();
	}
}

void BulletManager::Render(RenderContext& rc) {
	for (int i = 0;i < MAX_BULLET;i++) {
		if (m_bullets[i].active) {
			m_models[i].Draw(rc);
		}
	}
}