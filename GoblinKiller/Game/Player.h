#pragma once
#include "BulletManager.h"
class Player : public IGameObject
{
public:
	Player() {};
	~Player() {};

	bool Start()override;
	void Update()override;

	const Vector3& GetPosition() const { return m_position; }
	const Vector3& GetLoosDirection() const { return m_lookDir; }

	void Render(RenderContext& rc)override;

private:
	void Look();
	void Move();
	void UpdateCamera();

	void Fire();//射撃

	void ChangeWeapon();//武器の切り替え
	void FireRay(const Vector3& dir);//１本のレイを撃つ

	void UpdateReload();
	void StartReload();
	void UpdateAmmoText();

private:
	CharacterController m_charaCon;//キャラコン
	Vector3 m_position = Vector3::Zero;//足元の位置
	Vector3 m_moveSpeed = Vector3::Zero;//移動速度
	Vector3 m_lookDir = Vector3::AxisZ;//視線の向き
	float m_yaw = 0.0f;//左右の角度
	float m_pitch = 0.0f;//上下の角度

	SpriteRender m_crosshair;//標準
	FontRender m_hitText;//命中表示
	Vector3 m_eyePos = Vector3::Zero;//目の位置
	float m_fireTimer = 0.0f;//次に打てるまでの時間
	float m_hitTimer = 0.0f;//命中表示の残り時間

	int m_weaponNo = 0;//今持っている武器の番号
	FontRender m_weaponText;//武器名の表示

	BulletManager m_bulletManager;//オブジェクトプール用

	int m_ammo[3] = { 0,0,0 };//武器ごとの残弾（武器の数と同じ３つ）
	bool m_isReloading = false;//リロード中かの判定
	float m_reloadTimer = 0.0f;//リロードの残り時間
	FontRender m_ammoText;//弾数の表示
};

