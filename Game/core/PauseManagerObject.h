#pragma once

namespace app
{
	namespace core
	{
		class PauseManagerObject : public IGameObject
		{
		private:


		public:
			PauseManagerObject();
			~PauseManagerObject();
			
			bool Start();
			void Update();
			void Render(RenderContext& rc);
		};
	}
}

