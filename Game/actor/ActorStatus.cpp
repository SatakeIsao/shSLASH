/**
 * Actorファイル
 */
#include "stdafx.h"
#include "ActorStatus.h"
#include "core/ParameterManager.h"
#include "EnemyPhase.h"


namespace app
{
	namespace actor
	{


		void BattleCharacterStatus::LoadParameter(const char* path)
		{
			// 無し
		}


		void BattleCharacterStatus::Setup()
		{
			auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterBattleCharacterParameter>();

			moveSpeed_ = parameter->moveSpeed;
			jumpMoveSpeed_ = parameter->jumpMoveSpeed;
			jumpPower_ = parameter->jumpPower;
			radius_ = parameter->radius;
			height_ = parameter->height;
			hp_ = parameter->hp;
			currentHp_ = parameter->hp;
			attackPower_ = parameter->attackPower;
		}




		/**********************************/


		void EventCharacterStatus::LoadParameter(const char* path)
		{
			// 無し
		}

		void EventCharacterStatus::Setup()
		{
			auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterEventCharacterParameter>();

			moveSpeed_ = parameter->moveSpeed;
			jumpMoveSpeed_ = parameter->jumpMoveSpeed;
			jumpPower_ = parameter->jumpPower;
			radius_ = parameter->radius;
			height_ = parameter->height;
		}




		/**********************************/


		void MushroomEventCharacterStatus::LoadParameter(const char* path)
		{
			// 無し
		}

		void MushroomEventCharacterStatus::Setup()
		{
			auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterMushroomEventCharacterParameter>();

			moveSpeed_ = parameter->moveSpeed;
			jumpMoveSpeed_ = parameter->jumpMoveSpeed;
			jumpPower_ = parameter->jumpPower;
			radius_ = parameter->radius;
			height_ = parameter->height;
			hp_ = parameter->hp;
			currentHp_ = parameter->hp;
			attackPower_ = parameter->attackPower;

			enemyPhase_.SetPhases({
					{ 0,  parameter->hp, parameter->attackPower, parameter->moveSpeed},
					{ 4,  parameter->hp, parameter->attackPower * 1.5f, parameter->moveSpeed},
					{ 8,  parameter->hp, parameter->attackPower * 2.0f, parameter->moveSpeed},
				});
		}

		bool MushroomEventCharacterStatus::ApplyPhase(int playerLevel)
		{
			EnemyPhaseData data;
			if (enemyPhase_.Update(playerLevel, data))
			{
				// フェーズが進んだ場合
				hp_ = data.hp;
				currentHp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			// Reset直後(currentIndex_==-1)からの再適用時もここに来る可能性があるため
			// Updateが false でも現在フェーズのデータを取得して反映する
			if (enemyPhase_.TryGetCurrentPhaseData(data))
			{
				hp_ = data.hp;
				currentHp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			return false;
		}




		/**********************************/


		void StoneEventCharacterStatus::LoadParameter(const char* path)
		{
			// 無し
		}

		void StoneEventCharacterStatus::Setup()
		{
			auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterStoneEventCharacterParameter>();

			moveSpeed_ = parameter->moveSpeed;
			jumpMoveSpeed_ = parameter->jumpMoveSpeed;
			jumpPower_ = parameter->jumpPower;
			radius_ = parameter->radius;
			height_ = parameter->height;
			hp_ = parameter->hp;
			currentHp_ = parameter->hp;
			attackPower_ = parameter->attackPower;

			enemyPhase_.SetPhases({
					{ 0,  parameter->hp, parameter->attackPower, parameter->moveSpeed},
					{ 4,  parameter->hp, parameter->attackPower * 1.5f, parameter->moveSpeed},
					{ 8,  parameter->hp, parameter->attackPower * 2.0f, parameter->moveSpeed},
					});
		}

		bool StoneEventCharacterStatus::ApplyPhase(int playerLevel)
		{
			EnemyPhaseData data;
			if (enemyPhase_.Update(playerLevel, data))
			{
				// フェーズが進んだ場合
				hp_ = data.hp;
				currentHp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			// Reset直後(currentIndex_==-1)からの再適用時もここに来る可能性があるため
			// Updateが false でも現在フェーズのデータを取得して反映する
			if (enemyPhase_.TryGetCurrentPhaseData(data))
			{
				hp_ = data.hp;
				currentHp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			return false;
		}
	}
}