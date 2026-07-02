#pragma once

namespace app {

    struct GameResultData {
        int level = 0;
        int stoneKillCount = 0;
        int mushroomKillCount = 0;

        int CalcTotalScore() const {
            return level * 500 + stoneKillCount * 30 + mushroomKillCount * 50;
        }

        enum class Rank { Master = 0, Elite = 1, Beginner = 2 };
        Rank CalcRank() const {
            const int score = CalcTotalScore();
            if (score >= 6000) return Rank::Master;
            if (score >= 3000) return Rank::Elite;
            return Rank::Beginner;
        }

        static GameResultData& Get() {
            static GameResultData instance;
            return instance;
        }

        void Reset() {
            level = 0;
            stoneKillCount = 0;
            mushroomKillCount = 0;
        }
    };

}
