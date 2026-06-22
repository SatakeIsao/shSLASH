/**
 * IDamagePopListener.h
 *
 * ダメージ発生通知のインターフェース。
 * BattleManager（通知する側）と DamagePopPool（受け取る側）の
 * 両方がこのヘッダだけを include することで循環インクルードを回避する。
 */
#pragma once
#include "stdafx.h"

namespace app
{
    namespace battle
    {
        struct IDamagePopListener
        {
            virtual ~IDamagePopListener() {}
            virtual void OnDamageDealt(int damage, const Vector3& worldPos, bool isCritical = false) = 0;
        };
    }
}
