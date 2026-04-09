/**
 * CharacterSteeringファイル
 */
#include "stdafx.h"
#include "CharacterSteering.h"
#include "Actor.h"
#include "BattleCharacter.h"
#include "EventCharacter.h"


namespace
{
	template <typename T>
	void SetMoveDirection(T* target, const Vector3& direction)
	{
		// 未対応
	}
}


namespace app
{
	namespace actor
	{
		void CharacterSteering::Initialize(Character* target, const uint8_t index)
		{
			target_ = target;
			padIndex_ = index;
		}


		void CharacterSteering::Update()
		{
			auto inputVector = Vector3(GetPad()->GetLStickXF(), 0.0f, GetPad()->GetLStickYF());
			inputVector.Normalize();

			const bool isTriggerA = GetPad()->IsTrigger(enButtonA);
			const bool isPressA = GetPad()->IsPress(enButtonA);
			const bool isTriggerB = GetPad()->IsTrigger(enButtonB);
			const bool isTriggerDown = GetPad()->IsTrigger(enButtonDown);
			const bool isPressRB1 = GetPad()->IsPress(enButtonRB1);
			const bool isPressLB1 = GetPad()->IsPress(enButtonLB1);
			const bool isTriggerY = GetPad()->IsTrigger(enButtonY);

			// BattleCharacter
			{
				BattleCharacter* battleCharacter = dynamic_cast<BattleCharacter*>(target_);
				if (battleCharacter != nullptr) {
					auto* targetCharacterStateMachine = battleCharacter->GetStateMachine();
					targetCharacterStateMachine->SetActionA(isTriggerA);
					targetCharacterStateMachine->SetPressA(isPressA);
					targetCharacterStateMachine->SetActionB(isTriggerB);
					targetCharacterStateMachine->SetActionDown(isTriggerDown);
					targetCharacterStateMachine->SetActionRB1(isPressRB1);
					targetCharacterStateMachine->SetActionLB1(isPressLB1);
					targetCharacterStateMachine->SetTriggerY(isTriggerY);

					const bool isInput = inputVector.LengthSq() > MOVE_MIN_FLOAT;
					if (isInput)
					{
						targetCharacterStateMachine->SetMoveDirection(inputVector);
					}
					targetCharacterStateMachine->SetInputPower(isInput ? 1.0f : 0.0f);
				}
			}
		}
	}
}