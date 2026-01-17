# UniDx
Unity-like DirectX framework for learning purposes.
※ This project is primarily documented in Japanese.

UniDxは、UnityライクなDirectX用ゲームフレームワークです。


## 用途
- DirectX 入門（特にUnity経験者）
- ゲームフレームワークの構造理解に
- フレームワーク作成の参考・たたき台として


## 特徴
- GameObject、Component、Transform、Behaviourなど、Unityでおなじみの階層構造
- 簡易的な物理計算をサポート
- glTF形式の3Dモデルが使用可能
- 比較的コンパクトで理解しやすいコード


## 動作環境
- Windows 10/11
- Visual Studio 2022
- DirectX 11
- C++20


## 依存ライブラリ
- DirectXTK
- DirectXTex
- tinygltf


## サンプル実行手順

### 準備
このリポジトリは外部ライブラリを Git submodule として管理しています。  
clone 後、以下のコマンドをはじめに一度、実行してください。

```bash
git submodule update --init --recursive
```
または、このリポジトリ自体を以下のコマンドでcloneしても構いません。

```bash
git clone --recursive https://github.com/nittamas/UniDx.git
```

### 実行

/samples/UniDxSamples.sln を Visual Studio 2022 で開いてください。

ソリューションエクスプローラーで、Sample1_Sphereを右クリックして、
[スタートアップ プロジェクトに設定]を選択してください。

メニューから[デバッグ]-[デバッグの開始]を選択してください。


## Unityとの比較

### 座標系
- **座標系**: Unity と同じ左手座標系（Y-up、Z-forward）
- **行列レイアウト**: Row-Major（DirectX標準）
  - Unity は Column-Major

### 行列・クォータニオン演算の順序
```cpp
// UniDx: ベクトル * 行列（行ベクトル）
Vector3 worldPos = localPos * worldMatrix;

// UniDx: ベクトル * クォータニオン
Vector3 direction = localDir * rotation;

// Unity: 行列 * ベクトル（列ベクトル）
// Vector3 worldPos = worldMatrix * localPos;

// Unity: クォータニオン * ベクトル
// Vector3 direction = rotation * localDir;
```

### シェーダー内での行列演算
```hlsl
// UniDx (HLSL)
float4 worldPos = mul(float4(localPos, 1), world);
float4 clipPos  = mul(worldPos, view);
clipPos         = mul(clipPos, projection);

// Unity (ShaderLab)
// float4 clipPos = mul(UNITY_MATRIX_MVP, float4(localPos, 1));
```


## APIリファレンス
https://nittamas.github.io/UniDx/


## ライセンスについて

### UniDx 本体のソースコード
 MIT License

### サンプルリソース（3Dモデル・テクスチャ等）
 Creative Commons Attribution-ShareAlike 4.0 International
（CC BY-SA 4.0）

samples/resource 内のアセットは、著作者表記および継承（同一ライセンス配布）を
条件として改変および再配布が可能です。
詳しくは次のファイルを参照してください。
[samples/resource/LICENSE-CC-BY-SA.txt](samples/resource/LICENSE-CC-BY-SA.txt)

### フォント
サンプルで使用している M PLUS 1 フォント および
そこから変換したスプライトフォントは、

SIL Open Font License 1.1

のもとで提供されています。


※ UniDx は Unity Technologies および Microsoft と関係のない、独立したプロジェクトです。