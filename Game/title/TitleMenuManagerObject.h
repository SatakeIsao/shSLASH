#pragma once

namespace app
{
	namespace title
	{
		class TitleMenuManagerObject : public IGameObject
		{
		private:


		public:
			TitleMenuManagerObject();
			~TitleMenuManagerObject();
			
			bool Start();
			void Update();
			void Render(RenderContext& rc);
		};
	}
}

