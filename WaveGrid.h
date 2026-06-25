#pragma once
#include <vector>  // ← vectorを使うなら必要
#include <cmath>
struct SceneObject;

class WaveGrid {
public:

    WaveGrid(int width, int height, std::vector<SceneObject>& sceneObjects);
    ~WaveGrid();

    void update();
    void addSource(int x, int y, float strength);
    void setWall(int x, int y, bool isWall);        
    bool isWall(int x, int y);
    void setObjectWall(const std::vector<SceneObject>& objects);
    float getHeight(int x, int y);

    // ← これも追加しておくと便利
    void reset();
    int valueIndex(int x, int y) const;

    int mWidth, mHeight;

    struct Source {
        int x, y;
        float strength;
        float lifetime;  // 波源の寿命（減衰に使う）
    };

    std::vector<Source> mSources;
    std::vector<SceneObject>& sceneObjects;
    // float* よりvectorの方が管理が楽
    std::vector<float> mCurrent;
    std::vector<float> mPrevious;
    std::vector<float> mNext;    // ← 3フレーム必要
    std::vector<bool>  mWall;

    float mC;      // 波の速さ
    float mDeltaX; // グリッド間隔]

    // 既存のpublicの中に追加
    struct Normal {
        float x, y, z;
    };
    Normal getNormal(int x, int y);

};
