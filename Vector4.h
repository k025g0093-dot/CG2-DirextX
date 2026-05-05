#pragma once

struct Vector4 {
    float x, y, z, w;

    // コンストラクタ
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    // 色として使う時用の便利な別名（RGBA）
    // unionを使うと、x,y,z,w と r,g,b,a を同じメモリ領域で共有できます
    union {
        struct { float r, g, b, a; };
    };
};