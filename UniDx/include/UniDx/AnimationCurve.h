#pragma once
#include <vector>
#include <algorithm>
#include <cmath>

namespace UniDx {

// キーフレーム構造体
struct Keyframe {
    float time;         // キーフレームの時刻
    float value;        // キーフレームの値
    float inTangent;    // 入力接線（補間用）
    float outTangent;   // 出力接線（補間用）

    Keyframe(float t, float v, float inTan = 0.0f, float outTan = 0.0f)
        : time(t), value(v), inTangent(inTan), outTangent(outTan) {}
};

// アニメーションカーブクラス
class AnimationCurve
{
public:
    AnimationCurve() = default;
    explicit AnimationCurve(const std::vector<Keyframe>& keys) : keyframes(keys) {
        std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
            return a.time < b.time;
        });
    }

    // 任意のイテレータ範囲で初期化するコンストラクタ
    template <typename InputIt>
    AnimationCurve(InputIt first, InputIt last) : keyframes(first, last) {
        std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
            return a.time < b.time;
            });
    }

    // キーフレーム追加
    void AddKey(const Keyframe& key) {
        keyframes.push_back(key);
        std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
            return a.time < b.time;
        });
    }

    // 指定時刻で値を取得（線形補間）
    float Evaluate(float time) const;

    const std::vector<Keyframe>& GetKeys() const { return keyframes; }

private:
    std::vector<Keyframe> keyframes;
};

} // namespace UniDx