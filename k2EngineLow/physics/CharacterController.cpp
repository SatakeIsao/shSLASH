/*!
* @brief	キャラクタのコリジョンコントロール。
*/

#include "k2EngineLowPreCompile.h"
#include "CharacterController.h"

namespace nsK2EngineLow
{
    namespace
    {
        /** 地面判定 */
        struct SweepResultGround : public btCollisionWorld::ConvexResultCallback {
            bool isHit = false;
            Vector3 hitPos;
            Vector3 startPos;
            Vector3 hitNormal;
            btCollisionObject* me = nullptr;
            float dist = FLT_MAX;

            virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
                if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
                    return 1.0f;
                }

                Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
                float angle = acosf(hitNormalTmp.y); // 上(0,1,0)との角度

                // 地面判定 (法線が上を向いている)
                if (fabsf(angle) < Math::PI * 0.3f) {
                    isHit = true;
                    Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
                    Vector3 vDist = hitPosTmp - startPos;
                    float distTmp = vDist.Length();
                    if (dist > distTmp) {
                        hitPos = hitPosTmp;
                        hitNormal = hitNormalTmp;
                        dist = distTmp;
                    }
                }
                return 0.0f;
            }
        };


        /** 壁判定 */
        struct SweepResultWall : public btCollisionWorld::ConvexResultCallback {
            bool isHit = false;
            Vector3 hitPos;
            Vector3 startPos;
            Vector3 hitNormal;
            btCollisionObject* me = nullptr;
            float dist = FLT_MAX;

            virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
                if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
                    return 1.0f;
                }

                Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
                // 壁判定 (法線が横を向いている = 上との角度が大きい)
                float angle = fabsf(acosf(hitNormalTmp.y));
                if (angle >= Math::PI * 0.3f) {
                    isHit = true;
                    Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
                    Vector3 vDist = hitPosTmp - startPos;
                    vDist.y = 0.0f;
                    float distTmp = vDist.Length();
                    if (distTmp < dist) {
                        hitPos = hitPosTmp;
                        dist = distTmp;
                        hitNormal = hitNormalTmp;
                    }
                }
                return 0.0f;
            }
        };

        /** 天井用 */
        struct SweepResultCeiling : public btCollisionWorld::ConvexResultCallback
        {
            bool isHit = false;
            Vector3 hitPos;
            Vector3 startPos;
            btCollisionObject* me = nullptr;
            float dist = FLT_MAX;

            virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
            {
                if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
                    return 0.0f;
                }

                Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
                // 下方向と法線のなす角度をチェック、あるいは単純に y がマイナス（下向き）か
                if (hitNormalTmp.y < -0.5f) { // 法線が下を向いている＝天井
                    isHit = true;
                    Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
                    Vector3 vDist;
                    vDist.Subtract(hitPosTmp, startPos);
                    float distTmp = vDist.Length();
                    if (dist > distTmp) {
                        hitPos = hitPosTmp;
                        dist = distTmp;
                    }
                }
                return 0.0f;
            }
        };
    }




    /************************************************/


    CharacterController::CharacterController()
    {
    }


    CharacterController::~CharacterController()
    {
        RemoveRigidBoby();
    }


    void CharacterController::Init(float radius, float height, const Vector3& position)
    {
        position_ = position;
        prevPosition_ = position;
        radius_ = radius;
        height_ = height;
        collider_.Init(radius, height);

        RigidBodyInitData rbInfo;
        rbInfo.collider = &collider_;
        rbInfo.mass = 0.0f;
        rigidBody_.Init(rbInfo);

        btTransform& trans = rigidBody_.GetBody()->getWorldTransform();
        trans.setOrigin(btVector3(position.x, position.y + height_ * 0.5f + radius_, position.z));

        rigidBody_.GetBody()->setUserIndex(enCollisionAttr_Character);
        rigidBody_.GetBody()->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

        isInited_ = true;
    }

    void CharacterController::Jump(float jumpPower)
    {
        if (isOnGround_) {
            verticalVelocity_ = jumpPower;
            isOnGround_ = false;
            isJump_ = true;
        }
    }

    const Vector3& CharacterController::Execute(const Vector3& targetPosition, float deltaTime)
    {
        // 前フレームの座標を保存
        prevPosition_ = position_;

        // テレポートリクエストの確認
        if (isRequestTeleport_) {
            // テレポート処理
            // 物理演算をスキップして座標を強制適用

            position_ = targetPosition;

            // 内部物理状態のリセット
            verticalVelocity_ = 0.0f; // 落下の勢いなどを消す
            isOnGround_ = true;       // 安全のため一旦接地扱いにする（あるいは空中扱いにしたい場合はfalse）
            isJump_ = false;

            // フラグを消費して終了
            isRequestTeleport_ = false;
        }
        else {
            // 通常の物理移動処理

            // 重力の適用
            verticalVelocity_ += gravity_ * deltaTime;

            Vector3 nextPosition = position_;
            Vector3 intendedXZPos = targetPosition;
            intendedXZPos.y = position_.y; // Yは重力計算に任せるためここでは維持

            // XZ平面（壁）の移動解決
            {
                int loopCount = 0;
                Vector3 currentIterPos = position_;

                while (true) {
                    Vector3 moveDir = intendedXZPos - currentIterPos;
                    moveDir.y = 0.0f;
                    if (moveDir.Length() < FLT_EPSILON) {
                        nextPosition.x = intendedXZPos.x;
                        nextPosition.z = intendedXZPos.z;
                        break;
                    }

                    // SweepTest設定
                    Vector3 posTmp = currentIterPos;
                    posTmp.y += height_ * 0.5f + radius_ + height_ * 0.1f;

                    Vector3 start(posTmp.x, posTmp.y, posTmp.z);
                    Vector3 end(intendedXZPos.x, posTmp.y, intendedXZPos.z);
                    
                    SweepResultWall callback;
                    callback.me = rigidBody_.GetBody();
                    callback.startPos = posTmp;

                    PhysicsWorld::Get().ConvexSweepTest(collider_, start, end, callback);

                    if (callback.isHit) {
                        // 壁衝突：押し戻し計算
                        Vector3 vT0(intendedXZPos.x, 0.0f, intendedXZPos.z);
                        Vector3 vT1(callback.hitPos.x, 0.0f, callback.hitPos.z);
                        Vector3 vMerikomi = vT0 - vT1;

                        Vector3 hitNormalXZ = callback.hitNormal;
                        hitNormalXZ.y = 0.0f;
                        hitNormalXZ.Normalize();

                        float fT0 = hitNormalXZ.Dot(vMerikomi);
                        Vector3 vOffset = hitNormalXZ;
                        vOffset.Scale(-fT0 + radius_ + 0.001f);

                        intendedXZPos += vOffset;
                        currentIterPos = callback.hitPos;
                        currentIterPos.y = position_.y;
                    }
                    else {
                        nextPosition.x = intendedXZPos.x;
                        nextPosition.z = intendedXZPos.z;
                        break;
                    }
                    loopCount++;
                    if (loopCount >= 5) break;
                }
            }
            position_.x = nextPosition.x;
            position_.z = nextPosition.z;

            // Y軸（天井・床）の解決
            if (verticalVelocity_ > 0.0f) {
                // 上昇中（天井判定）
                float upAmount = verticalVelocity_ * deltaTime;
                float checkY = position_.y + height_ * 0.5f + radius_;

                Vector3 start(position_.x, checkY, position_.z);
                Vector3 end(position_.x, checkY + upAmount, position_.z);

                SweepResultCeiling callback; // ※前回の定義を参照
                callback.me = rigidBody_.GetBody();
                callback.startPos = position_;
                callback.startPos.y = checkY;

                if ((start - end).LengthSq() >= 0.01f) {
                    PhysicsWorld::Get().ConvexSweepTest(collider_, start, end, callback);
                }
                if (callback.isHit) {
                    verticalVelocity_ = 0.0f;
                    float dist = (callback.hitPos.y - checkY);
                    if (dist < 0) dist = 0;
                    position_.y += dist - 0.01f;
                }
                else {
                    position_.y += upAmount;
                }
                isOnGround_ = false;
            }
            else
            {
                // 落下中（床判定）
                float downAmount = fabsf(verticalVelocity_ * deltaTime);
                float checkDist = (isOnGround_) ? 0.5f : downAmount + 0.1f;
                float checkY = position_.y + height_ * 0.5f + radius_;

                Vector3 start(position_.x, checkY, position_.z);
                Vector3 end(position_.x, checkY - checkDist, position_.z);

                SweepResultGround callback;
                callback.me = rigidBody_.GetBody();
                callback.startPos = Vector3(position_.x, checkY, position_.z);

                PhysicsWorld::Get().ConvexSweepTest(collider_, start, end, callback);

                if (callback.isHit) {
                    isOnGround_ = true;
                    isJump_ = false;
                    verticalVelocity_ = 0.0f;
                    position_.y = callback.hitPos.y;
                }
                else {
                    isOnGround_ = false;
                    position_.y -= downAmount;
                }
            }
        }

        // 剛体（Collider）の位置を更新
        // テレポート時も通常時も、最終的な m_position を反映させる
        btRigidBody* btBody = rigidBody_.GetBody();
        btBody->setActivationState(DISABLE_DEACTIVATION);
        btTransform& trans = btBody->getWorldTransform();
        trans.setOrigin(btVector3(position_.x, position_.y + height_ * 0.5f + radius_, position_.z));

        return position_;
    }


    void CharacterController::RemoveRigidBoby()
    {
        PhysicsWorld::Get().RemoveRigidBody(rigidBody_);
    }
}
