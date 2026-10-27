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

	struct WeaponData {
		const wchar_t* name; //表示名
		float fireInterval;//連射間隔
		int damage;//１発のダメージ
		int pellets;//１回に出る段数（ショットガン用）
		float spread;//弾のばらつき
		int magazine;//マガジンの大きさ
		float reloadTime;//リロード時間（秒）
	};

	const int WEAPON_NUM = 3;
	const WeaponData WEAPONS[WEAPON_NUM] = {
		{L"RIFLE",0.10f,10,1,0.01f,30,1.5f},
		{L"SHOTGUN",0.80f,8,8,0.08f,6,2.5f},
		{L"SNIPER",1.50f,100,1,0.00f,5,2.0f},
	};

	//-1.0~1.0のランダムな値
	float RandomSigned() {
		return(static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
	}


	//視点操作の調整
	const float LOOK_DEADZONE = 0.15f;//これ以下の倒し具合は無視する
	const float LOOK_CURVE = 2.0f;//大きいほど、少し倒したときの動きが遅くなる

	//右スティックの倒し具合を0.0~1.0のなめらかな値で取得する
	//パッドがつながっていない時はfalseを返す
	bool GetAnalogRStick(float& outX, float& outY) {
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(state));
		if (XInputGetState(0, & state) != ERROR_SUCCESS) {
			return false;//未接続
		}

		//-32768~32767を-1.0~1.0に治す
		float x = state.Gamepad.sThumbRX / 32767.0f;
		float y = state.Gamepad.sThumbRY / 32767.0f;
		if (x > 1.0f) { x = 1.0f; }
		if (x < -1.0f) { x = -1.0f; }
		if (y > 1.0f) { y = 1.0f; }
		if (y < -1.0f) { y = -1.0f; }

		//倒した長さ
		float length = sqrtf(x * x + y * y);
		if (length < LOOK_DEADZONE) {
			outX = 0.0f;
			outY = 0.0f;
			return true;//デッドゾーン内
		}

		//デッドゾーンを引いて、0.0~1.0に伸ばし直す
		float rate = (length - LOOK_DEADZONE) / (1.0f - LOOK_DEADZONE);
		if (rate > 1.0f) { rate = 1.0f; }

		//カーブをかける（2乗すると、小さい入力ほど、より小さく）
		rate = powf(rate, LOOK_CURVE);

		//向きは変えず、長さだけrateにする
		outX = x / length * rate;
		outY = y / length * rate;
		return true;
	}
}



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


	m_weaponText.SetText(WEAPONS[m_weaponNo].name);
	m_weaponText.SetPosition(300.0f, -300.0f, 0.0f);

	m_bulletManager.Init();

	//すべての武器をマガジン満タンで始める
	for (int i = 0;i < WEAPON_NUM;i++) {
		m_ammo[i] = WEAPONS[i].magazine;
	}

	m_ammoText.SetPosition(300.0f, -340.0f, 0.0f);//武器名の下
	UpdateAmmoText();

	return true;
}

void Player::Update() {
	//順番が大事:向きを決める->その向きで移動->最後にカメラへ反映
	Look();
	Move();
	UpdateCamera();

	ChangeWeapon();
	UpdateReload();

	//Gun
	float dt = g_gameTime->GetFrameDeltaTime();
	m_fireTimer -= dt;
	m_hitTimer -= dt;

	

	//右トリガーを押している間、一定間隔で撃つ
	if (g_pad[0]->IsPress(enButtonRB2) && m_fireTimer <= 0.0f && !m_isReloading) {
		if (m_ammo[m_weaponNo] > 0) {
			Fire();
			m_ammo[m_weaponNo]--;//１発消費
			m_fireTimer = WEAPONS[m_weaponNo].fireInterval;
			UpdateAmmoText();

			if (m_ammo[m_weaponNo] == 0) {
				StartReload();
			}
		}
		else {
			StartReload();
		}
		
		/*Fire();
		m_fireTimer = WEAPONS[m_weaponNo].fireInterval;*/
		//m_fireTimer = FIRE_INTERVAL;
	}


	m_bulletManager.Update();

}

void Player::Look() {
	float dt = g_gameTime->GetFrameDeltaTime();

	float stickX = 0.0f;
	float stickY = 0.0f;
	if (!GetAnalogRStick(stickX, stickY)) {
		//パッドがない時は、今まで道理エンジンの値を使う
		stickX = g_pad[0]->GetRStickXF();
		stickY = g_pad[0]->GetRStickYF();
	}

	m_yaw += stickX * LOOK_SPEED * dt;
	m_pitch += stickY * LOOK_SPEED * dt;


	//m_yaw += g_pad[0]->GetRStickXF() * LOOK_SPEED * dt;
	//m_pitch += g_pad[0]->GetRStickYF() * LOOK_SPEED * dt;

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

	const WeaponData& weapon = WEAPONS[m_weaponNo];

	//視線に垂直な「右」と「上」の方向（ばらつき用）
	Vector3 right(cosf(m_yaw), 0.0f, -sinf(m_yaw));
	Vector3 up(-sinf(m_pitch) * sinf(m_yaw), cosf(m_pitch), -sinf(m_pitch) * cosf(m_yaw));

	//弾の数だけレイを撃つ
	for (int i = 0;i < weapon.pellets;i++) {
		Vector3 dir = m_lookDir
			+ right * (RandomSigned() * weapon.spread)
			+ up * (RandomSigned() * weapon.spread);
		dir.Normalize();
		FireRay(dir);
	}

}

void Player::FireRay(const Vector3& dir) {
	//１．壁・床までの距離を調べる
	float wallDist = FIRE_RANGE;
	Vector3 rayStart = m_eyePos + dir * RAY_OFFSET;
	Vector3 rayEnd = m_eyePos + dir * FIRE_RANGE;
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
		if (target->IntersectRay(m_eyePos, dir, dist) && dist < nearestDist) {
			nearestDist = dist;
			nearest = target;
		}
		return true;
		});

	//３．当たったらダメージ
	if (nearest != nullptr) {
		//nearest->TakeDamage(FIRE_DAMAGE);

		nearest->TakeDamage(WEAPONS[m_weaponNo].damage);
		m_hitTimer = 0.2f;
	}

	//４．見た目の玉を飛ばす
	//Vector3 end = m_eyePos + dir * nearestDist;

	//重工の位置：目から少し右・下・前にずらす
	/*Vector3 right(cosf(m_yaw), 0.0f, -sinf(m_yaw));
	Vector3 muzzle = m_eyePos + right * 15.0f + Vector3(0.0f, -12.0f, 0.0f) + m_lookDir * 40.0f;*/

	Vector3 end = m_eyePos + dir * nearestDist;
	Vector3 right(cosf(m_yaw), 0.0f, -sinf(m_yaw));
	Vector3 muzzle = m_eyePos + right * 15.0f + Vector3(0.0f, -12.0f, 0.0f) + m_lookDir * 40.0f;

	m_bulletManager.Spawn(muzzle, end);
}

void Player::ChangeWeapon() {
	int old = m_weaponNo;

	if (g_pad[0]->IsTrigger(enButtonRB1)) {
		m_weaponNo = (m_weaponNo + 1) % WEAPON_NUM;
	}
	if (g_pad[0]->IsTrigger(enButtonLB1)) {
		m_weaponNo = (m_weaponNo + WEAPON_NUM - 1) % WEAPON_NUM;
	}

	if (m_weaponNo != old) {
		m_weaponText.SetText(WEAPONS[m_weaponNo].name);
		m_isReloading = false;
		UpdateAmmoText();
	}
}

void Player::StartReload() {
	if (m_isReloading) {
		return;//すでにリロード中
	}
	if (m_ammo[m_weaponNo] >= WEAPONS[m_weaponNo].magazine) {
		return;//マガジンが満タンなら不要
	}
	m_isReloading = true;
	m_reloadTimer = WEAPONS[m_weaponNo].reloadTime;
	UpdateAmmoText();
}

void Player::UpdateReload() {
	if (!m_isReloading) {
		//手動リロード（Xbutton）
		if (g_pad[0]->IsTrigger(enButtonX)) {
			StartReload();
		}
		return;
	}

	//リロード中：時間がたったらマガジンを満タンに知る
	m_reloadTimer -= g_gameTime->GetFrameDeltaTime();
	if (m_reloadTimer <= 0.0) {
		m_ammo[m_weaponNo] = WEAPONS[m_weaponNo].magazine;//予備弾は無限
		m_isReloading = false;
		UpdateAmmoText();
	}
}

void Player::UpdateAmmoText() {
	wchar_t text[64];
	if (m_isReloading) {
		swprintf_s(text, L"RELOADING...");
	}
	else {
		swprintf_s(text, L"%d/%d", m_ammo[m_weaponNo], WEAPONS[m_weaponNo].magazine);
	}
	m_ammoText.SetText(text);
}



void Player::Render(RenderContext& rc) {
	m_crosshair.Draw(rc);
	m_weaponText.Draw(rc);
	if (m_hitTimer > 0.0f) {
		m_hitText.Draw(rc);
	}
	m_bulletManager.Render(rc);
	m_ammoText.Draw(rc);
}