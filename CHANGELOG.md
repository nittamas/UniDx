## [0.3.0] - 2026-08-15

### Added
- GameObject の Instantiate 機能を追加しました。
- Timeに最大許容時間ステップを追加しました。

### Changed
- Property の代入は値のセットとし、コピーは削除しました。
- GetComponentInParent() が祖先をたどるようにしました。

### Fixed
- Releaseビルドのリンクエラーを修正しました。
- 実行中の Component 追加時のエラーを修正しました。

### Breaking Changes
- GetComponent の引数を削除しました。

---

## [0.2.0] - 2026-01-18

### Added
- スキンメッシュモデルの描画機能を実装しました。
- インターンプールを使った StringId を追加しました。

### Changed
- 物理衝突検出に階層型グリッドを使うようにしました。

### Fixed
- 座標系の不整合を修正しました。

### Breaking Changes
- UTF-8文字コードを標準とし、通常使う文字列をu8stringへ変更しました。
  これまで L"" を指定していた箇所は u8"" に変更してください。

---

## [0.1.0] - 2025-12-28

### Added
- UniDx の初期バージョンを公開しました。
- 基本的な GameObject / Component / Transform 構成を実装しました。
- DirectX11 を用いた最小限の描画パイプラインを実装しました。
- glTF形式の3Dモデルの読み込み機能を実装しました。
- 簡易的な物理計算機能を実装しました。