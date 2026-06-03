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

			std::vector<EnemyPhaseData> phaseData;
			for (const auto& phase : parameter->phases)
			{
				phaseData.push_back({ phase.requiredPlayerLevel, parameter->hp, phase.attackPower, parameter->moveSpeed });
			}
			enemyPhase_.SetPhases(phaseData);
			attackPower_ = phaseData.empty() ? 0.0f : phaseData[0].attackPower;
		}

		bool MushroomEventCharacterStatus::ApplyPhase(int playerLevel)
		{
			EnemyPhaseData data;
			if (enemyPhase_.Update(playerLevel, data))
			{
				// フェーズが進んだ場合：攻撃力・移動速度のみ更新（HPはリセットしない）
				hp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			// フェーズ遷移なし：攻撃力・移動速度のみ反映（HPはリセットしない）
			if (enemyPhase_.TryGetCurrentPhaseData(data))
			{
				hp_ = data.hp;
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

			std::vector<EnemyPhaseData> phaseData;
			for (const auto& phase : parameter->phases)
			{
				phaseData.push_back({ phase.requiredPlayerLevel, parameter->hp, phase.attackPower, parameter->moveSpeed });
			}
			enemyPhase_.SetPhases(phaseData);
			attackPower_ = phaseData.empty() ? 0.0f : phaseData[0].attackPower;
		}

		bool StoneEventCharacterStatus::ApplyPhase(int playerLevel)
		{
			EnemyPhaseData data;
			if (enemyPhase_.Update(playerLevel, data))
			{
				// フェーズが進んだ場合：攻撃力・移動速度のみ更新（HPはリセットしない）
				hp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			// フェーズ遷移なし：攻撃力・移動速度のみ反映（HPはリセットしない）
			if (enemyPhase_.TryGetCurrentPhaseData(data))
			{
				hp_ = data.hp;
				attackPower_ = data.attackPower;
				moveSpeed_ = data.moveSpeed;
				return true;
			}

			return false;
		}
	}
}