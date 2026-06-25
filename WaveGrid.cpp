#include "WaveGrid.h"
#include "TUFEngine.h"
#include <algorithm>

WaveGrid::WaveGrid(int width, int height, std::vector<SceneObject>& sceneObjects)
    : mWidth(width), mHeight(height), sceneObjects(sceneObjects), mC(40.0f), mDeltaX(1.0f)
{
    mCurrent.resize(mWidth * mHeight, 0.0f);
    mPrevious.resize(mWidth * mHeight, 0.0f);
    mNext.resize(mWidth * mHeight, 0.0f);
    mWall.resize(mWidth * mHeight, false);

}

WaveGrid::~WaveGrid() {
    // vectorは自動で解放されるので何も書かなくていい
}

void WaveGrid::update() {
    // 各波源を処理
    for (auto& s : mSources) {
        // 波源の位置に強度を加える
        int index = s.y * mWidth + s.x;
        mCurrent[index] += s.strength * s.lifetime;

        // 寿命を減らす（減衰）
        s.lifetime -= 0.05f;
    }

    // 寿命が尽きた波源を削除
    mSources.erase(
        std::remove_if(mSources.begin(), mSources.end(),
            [](const Source& s) { return s.lifetime <= 0.0f; }),
        mSources.end()
    );

    setObjectWall(sceneObjects);

    // 波の更新（簡略化した例）
    for (int y = 1; y < mHeight - 1; ++y) {
        for (int x = 1; x < mWidth - 1; ++x) {
            int index = y * mWidth + x;
            if (mWall[index]) {
                mNext[index] = 0.0f; // 壁は波を伝えない
            }
            else {
                float deltaT = 1.0f / 60.0f;
                float mul = deltaT * deltaT * mC * mC / (mDeltaX * mDeltaX);

                float u = mCurrent[index];
                float uPre = mPrevious[index];
                float uL = mCurrent[index - 1];
                float uR = mCurrent[index + 1];
                float uT = mCurrent[index - mWidth];
                float uB = mCurrent[index + mWidth];
                // 減衰係数（1.0に近いほどゆっくり消える）
                float damping = 0.993f;

                mNext[index] = damping * (u + u - uPre + mul * (-4.0f * u + uL + uR + uT + uB));
            }
        }
    }
    // フレームをローテーション
    std::swap(mPrevious, mCurrent);
    std::swap(mCurrent, mNext);

}

// addSource の実装
void WaveGrid::addSource(int x, int y, float strength) {
    Source s;
    s.x = x;
    s.y = y;
    s.strength = strength;
    s.lifetime = 1.0f;  // 最大1.0からスタート
    mSources.push_back(s);  // vectorに追加
}

void WaveGrid::setObjectWall(const std::vector<SceneObject>& objects) {
    std::fill(mWall.begin(), mWall.end(), false);
    for (const auto& object : objects) {

        // ---- Y軸交差判定: localAABB + scale を使って正確に ----
        float worldMinY = object.pos.y + object.localAABB.min.y * object.scale.y;
        float worldMaxY = object.pos.y + object.localAABB.max.y * object.scale.y;
        if (worldMinY > 0.0f || worldMaxY < 0.0f) {
            continue;
        }

        // ---- XZ OBB判定: 回転に対応 ----
        // OBB中心をグリッド座標に変換
        float cx = object.obb.center.x / mDeltaX + mWidth * 0.5f;
        float cz = object.obb.center.z / mDeltaX + mHeight * 0.5f;

        // OBBのX軸・Z軸（XZ平面に投影、すでに正規化済み）
        float axx = object.obb.orientations[0].x;
        float axz = object.obb.orientations[0].z;
        float azx = object.obb.orientations[2].x;
        float azz = object.obb.orientations[2].z;

        // 半サイズ（グリッド単位、既にscale適用済み）
        float hx = object.obb.size.x / mDeltaX;
        float hz = object.obb.size.z / mDeltaX;

        // OBBを囲むワールドAABB（グリッド座標）を計算して走査範囲を絞る
        float extentX = fabsf(axx * hx) + fabsf(azx * hz);
        float extentZ = fabsf(axz * hx) + fabsf(azz * hz);

        int minX = (std::max)(0, (int)floorf(cx - extentX));
        int maxX = (std::min)(mWidth - 1, (int)ceilf(cx + extentX));
        int minZ = (std::max)(0, (int)floorf(cz - extentZ));
        int maxZ = (std::min)(mHeight - 1, (int)ceilf(cz + extentZ));

        for (int z = minZ; z <= maxZ; z++) {
            for (int x = minX; x <= maxX; x++) {
                float gx = (float)x - cx;
                float gz = (float)z - cz;

                // グリッド中心をOBBの各軸に射影 → 半サイズ内なら壁
                float projX = fabsf(gx * axx + gz * axz);
                float projZ = fabsf(gx * azx + gz * azz);

                if (projX <= hx && projZ <= hz) {
                    setWall(x, z, true);
                }
            }
        }
    }
}


void WaveGrid::setWall(int x, int y, bool isWall) {
    mWall[y * mWidth + x] = isWall;
}


float WaveGrid::getHeight(int x, int y) {
    return mCurrent[y * mWidth + x];  // グリッドの値を返すだけ

}

void WaveGrid::reset() {
    std::fill(mCurrent.begin(), mCurrent.end(), 0.0f);
    std::fill(mPrevious.begin(), mPrevious.end(), 0.0f);
    std::fill(mNext.begin(), mNext.end(), 0.0f);
    mSources.clear();
}

int WaveGrid::valueIndex(int x, int y) const {
    return y * mWidth + x;
}

WaveGrid::Normal WaveGrid::getNormal(int x, int y) {
    // 端の処理（範囲外アクセス防止）
    int left = x > 0 ? x - 1 : x;
    int right = x < mWidth - 1 ? x + 1 : x;
    int up = y > 0 ? y - 1 : y;
    int down = y < mHeight - 1 ? y + 1 : y;

    float hL = getHeight(left, y);
    float hR = getHeight(right, y);
    float hU = getHeight(x, up);
    float hD = getHeight(x, down);

    float nx = hL - hR;
    float ny = 2.0f;
    float nz = hU - hD;

    // 正規化
    float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len > 0.0f) {
        nx /= len;
        ny /= len;
        nz /= len;
    }

    return { nx, ny, nz };
}
