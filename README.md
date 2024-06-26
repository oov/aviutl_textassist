# テキスト編集補助プラグイン

AviUtl の拡張編集プラグインで複数行テキストを編集する際に、`Alt + 方向キー` で制御文字の内容を書き換える AviUtl 用プラグインです。

## 注意事項

テキスト編集補助プラグインは無保証で提供されます。  
テキスト編集補助プラグインを使用したこと及び使用しなかったことによるいかなる損害について、開発者は何も責任を負いません。

これに同意できない場合、あなたはテキスト編集補助プラグインを使用することができません。

## インストール／アンインストール

`textassist.auf` を `exedit.auf` と同じ場所に入れてください。

アンインストールは導入したファイルと `textassist.ini` を削除するだけで完了です。  
設定を一度も変更していなければ `textassist.ini` は存在しません。

## 使い方

拡張編集上でテキストオブジェクトを作成し、そのテキスト入力欄でショートカットキーを押すと制御文字の `挿入` と `書き換え` ができます。  
それぞれの機能は以下で解説します。

### 挿入

`Alt + T` を押すと、キャレット位置にポップアップメニューが表示されます。  
項目を選ぶとキャレット位置に制御文字を挿入できます。

また、範囲選択をした状態で `Alt + T` を押すと選択範囲を前後に制御文字が出力されます。  
この場合は「閉じ」が存在する `色の変更`、`フォント`、`表示速度` のみが利用可能です。

なお、もし PSDToolKit がインストールされている場合は PSDToolKit 用の制御文字 `<ss>` や `<pp>` も挿入できます。

### 移動

制御文字の内側にキャレットを置いた状態で `Alt + ←` や `Alt + →` などを押すと、制御文字の値ごとにカーソルを移動できます。

例えば `<s48|,MS UI Gothic>` のようにフォントサイズのところにキャレットがある状態で `Alt + →` を押すと `<s48,MS UI Gothic|>` のようにフォント名に移動できます。

### 書き換え

制御文字の内側にキャレットを置いた状態で `Alt + ↓` や `Alt + ↑` などを押すと、制御文字の値を書き換えられます。  
書き換えできるのは以下のような要素です。

#### 色の変更

R, G, B の各パートの値を増減させられます。  
例えば `<#fffff|f>` この位置にキャレットを置いて `Alt + ↓` を押すと B の成分を変更できます。  
そのままだと1ずつ、Shift キーを押しながらだと16ずつ増減できます。

キャレット位置と成分の対応は以下の通りです。

- R `<#f|fffff>`
- G `<#fff|fff>`
- B `<#fffff|f>`

#### サイズ、フォントの変更

サイズ、フォント名、スタイルのすべてを `Alt + ↓` や `Alt + ↑` で変更できます。  
キャレットをどの位置に置くのかによって変更する項目を選ぶことができます。

また、フォント名を `<s48,MS UI|,BI>` のように不完全に入力して `Alt + ↓` などを押すと、近い名前のフォント名の候補一覧を表示できます。

#### 座標の指定

キャレットが内側のどの位置にあっても、`Alt + ←` と `Alt + →` でX座標、`Alt + ↑` と `Alt + ↓` でY座標を変更できます。  
そのままだと1ずつ、Shift キーを押しながらだと10ずつ増減できます。

また、制御文字の内側ではない場所で `Alt + 方向キー` を押すと、座標の相対位置での座標の指定制御文字 `<p+0,+0>` が挿入されます。  
これはカーニングの用途を意図したものです。

#### 表示速度の変更 / 表示ウェイト / 表示クリア

`Alt + ↑` と `Alt + ↓` で0.1ずつ、Shift キーを押しながらだと1ずつ変更できます。

## オプション

AviUtl のメインメニューから `表示` → `テキスト編集補助` を選ぶとオプション設定が表示されます。

ここで変更した内容は `textassist.auf` と同じ場所に `textassist.ini` として保存されます。

### `Alt+方向キーで <p> より <pp> を優先する`

このオプションを有効にすると、制御文字がない場所で `Alt + 方向キー` を押したときに `<p>` ではなく `<pp>` を挿入するようになります。  
このオプションはデフォルトでは無効で、PSDToolKit がインストールされていなければ有効にしても効果がありません。

更新履歴
--------

[CHANGELOG.md](https://github.com/oov/aviutl_textassist/CHANGELOG.md) を参照してください。

Contributors
------------

- sigma-axis

Credits
-------

テキスト編集補助プラグイン is made possible by the following open source softwares.

### [Acutest](https://github.com/mity/acutest)

<details>
<summary>The MIT License</summary>

```
The MIT License (MIT)

Copyright © 2013-2019 Martin Mitáš

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
```
</details>

### [AviUtl Plugin SDK](http://spring-fragrance.mints.ne.jp/aviutl/)

<details>
<summary>The MIT License</summary>

```
The MIT License

Copyright (c) 1999-2012 Kenkun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
</details>

### [onp](https://github.com/convto/onp)

<details>
<summary>The MIT License</summary>

```
MIT License

Copyright (c) 2019 YuyaOkumura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
</details>
