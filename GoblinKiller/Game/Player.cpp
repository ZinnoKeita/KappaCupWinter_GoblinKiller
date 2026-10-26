#include "stdafx.h"
#include "Player.h"
#include "Target.h"

namespace {
	//Playerの動き
	const float EYE_HEIGHT = 160.0f;//目の高さ
	const float MOVE_SPEED = 250.0f;//移動速度
	const float LOOK_SPEED = 2.5f;//視点の回転速度
	const float GRAVITY = 980.0f;//重力
	const float PITCH_LIMIT = 80.0f;//上下に向ける最大角度

	//射撃に関して
	const float FIRE_INTERVAL = 0.15f;//射撃間隔（秒）
	const float FIRE_RANGE = 5000.0f;//射程
	const int FIRE_DAMAGE = 25;//1発のダメージ
	const float RAY_OFFSET = 30.0f;//自分の当たり判定を避けるため、レイを少し前から始める
}

//struct WeaponData {
//	const wchar_t* name; //表示名
//	float fireInterval;//連射間隔
//	int damage;//１発のダメージ
//	int pellets;//１回に出る段数（ショットガン用）
//	float spread;//弾のばらつき
//};

bool Player::Start() {
	//Player
	m_position = Vector3(0.0f, 0.0f, 0.0f);

	//キャラコンの初期化
	m_charaCon.Init(25.0f, 120.0f, m_position);

	//近いものが見切れないように近平面を小さくする
	g_camera3D->SetNear(1.0f);
	g_camera3D->SetFar(10000.0f);


	//Gun
	m_crosshair.Init("Assets/sprite/maruten.dds", 64.0f, 64.0f);
	m_crosshair.SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	m_crosshair.Update();

	m_hitText.SetText(L"HIT");
	m_hitText.SetPosition(40.0f, -30.0f, 0.0f);
	m_hitText.SetColor(1.0f, 0.2f, 0.2f, 1.0f);


	return true;
}

void Player::Update() {
	//順番が大事:向きを決める->その向きで移動->最後にカメラへ反映
	Look();
	Move();
	UpdateCamera();

	//Gun
	float dt = g_gameTime->GetFrameDeltaTime();
	m_fireTimer -= dt;
	m_hitTimer -= dt;

	//右トリガーを押している間、一定間隔で撃つ
	if (g_pad[0]->IsPress(enButtonRB2) && m_fireTimer <= 0.0f) {
		Fire();
		m_fireTimer = FIRE_INTERVAL;
	}

}

void Player::Look() {
	float dt = g_gameTime->GetFrameDeltaTime();

	m_yaw += g_pad[0]->GetRStickXF() * LOOK_SPEED * dt;
	m_pitch += g_pad[0]->GetRStickYF() * LOOK_SPEED * dt;

	//真上・ましたを向いてひっくり返らないように制限
	const float limit = Math::DegToRad(PITCH_LIMIT);
	if (m_pitch > limit) { m_pitch = limit; }
	if (m_pitch < -limit) { m_pitch = -limit; }
}

void Player::Move() {
	float dt = g_gameTime->GetFrameDeltaTime();

	//移動には「水平方向の向き」だけを使う（上を向いても空には飛ばない）
	Vector3 forward(sinf(m_yaw), 0.0f, cosf(m_yaw));
	Vector3 right(cosf(m_yaw), 0.0f, -sinf(m_yaw));

	float stickX = g_pad[0]->GetLStickXF();
	float stickY = g_pad[0]->GetLStickYF();

	//水平方向の速度(Yは重力用に残しておく)
	Vector3 horizontal = forward * (stickY * MOVE_SPEED) + right * (stickX * MOVE_SPEED);

	m_moveSpeed.x = horizontal.x;
	m_moveSpeed.z = horizontal.z;

	//重力
	if (m_charaCon.IsOnGround()) {
		m_moveSpeed.y = 0.0f;
	}
	else {
		m_moveSpeed.y -= GRAVITY * dt;
	}

	m_position = m_charaCon.Execute(m_moveSpeed, dt);
}

void Player::UpdateCamera() {
	//角度から「向いている方向」を計算
	m_lookDir.x = cosf(m_pitch) * sinf(m_yaw);
	m_lookDir.y = sinf(m_pitch);
	m_lookDir.z = cosf(m_pitch) * cosf(m_yaw);

	//目の位置にカメラを置いて、向いている方向を見る
	m_eyePos = m_position;
	m_eyePos.y += EYE_HEIGHT;

	g_camera3D->SetPosition(m_eyePos);
	g_camera3D->SetTarget(m_eyePos + m_lookDir);
}

void Player::Fire() {
	//１．壁・床までの距離を調べる
	float wallDist = FIRE_RANGE;
	Vector3 rayStart = m_eyePos + m_lookDir * RAY_OFFSET;
	Vector3 rayEnd = m_eyePos + m_lookDir * FIRE_RANGE;
	Vector3 hitPos;
	if (PhysicsWorld::GetInstance()->RayTest(rayStart, rayEnd, hitPos)) {
		wallDist = (hitPos - m_eyePos).Length();
	}

	//２．的の中で、一番近くて、壁より手前の物を探す
	Target* nearest = nullptr;
	float nearestDist = wallDist;

	QueryGOs<Target>("target", [&](Target* target) {
		if (target == nullptr || target->IsDead()) {
			return true;//次へ
		}
		float dist = 0.0f;
		if (target->IntersectRay(m_eyePos, m_lookDir, dist) && dist < nearestDist) {
			nearestDist = dist;
			nearest = target;
		}
		return true;
		});

	//３．当たったらダメージ
	if (nearest != nullptr) {
		nearest->TakeDamage(FIRE_DAMAGE);
		m_hitTimer = 0.2f;
	}
}

void Player::Render(RenderContext& rc) {
	m_crosshair.Draw(rc);
	if (m_hitTimer > 0.0f) {
		m_hitText.Draw(rc);
	}
}