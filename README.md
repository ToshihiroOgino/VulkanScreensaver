# Vulkan Screensaver

## 環境

Ubuntu 22.04にて開発やテストなどを行っています。

コンパイルには依存関係のリポジトリと、いくつかのライブラリが必要です。
詳しくはこのサイトを確認してください。
<https://vulkan-tutorial.com/Development_environment>

```bash
# 依存リポジトリのダウンロード
git submodule update --init --recursive --recommend-shallow --depth 1
# ライブラリのインストール
apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev libglm-dev libglfw3-dev
```

## コンパイル・実行

シェダーはglslcを利用してコンパイルしてください。リポジトリのルートディレクトリにあるシェルスクリプトを利用することで、shadersフォルダ内のシェダーをコンパイルすることができます。

```bash
./CompileShaders.sh
```

以下のコマンドから、コードのコンパイルと実行を行えます。

```bash
make
make run
```

## スクリーンセーバー記述言語概要

文法の定義は以下の通りです。
角度は全て度数法を用いて記述します。
数字は符号付き32ビット浮動小数点数に変換されます。
入力コード中に含まれる、空白や改行、その他の意味が未定義の文字は全て無視されます。

```plaintext
<code> ::= (<obj>)*

<obj> ::= <node> | <camera>

<node> ::= 'node{' ( <nodeParam> | ('state{' <stateParam>* '};') | <obj>)* '};'
<nodeParam> ::= ('time=' <number> | ('position' | 'rotation' | 'scale') '=' <vector3> )';'
<stateParam> ::= (('time=' <number>) | (('position' | 'rotation' | 'scale') '=' <vector3> ))';'

<camera> ::= { (<cameraParam>)+ };
<cameraParam> ::= ('position' | 'lookAt' | 'angularVelocity') '=' <vector3> ';'

<number> ::= ('-')? (0-9)+ ('.' (0-9)+)?
<vector3> ::= '(' <number> ',' <number> ',' <number> ')'
<resourcePathSet> ::= '( "' ((A-z)|'/'|'_'|'.')+  '" , "' ((A-z)|'/'|'_'|'.')+ '")'
```

宣言されたオブジェクトに内で初期化されていない変数は規定の初期値が使用されます。

node内のstateはnodeの動きを定義します。
stateは記述されているノード以下全てのノードの状態に影響します。
time秒間かけて、次のstateに遷移します。なお、stateが1つ以下しか宣言されていない場合には状態の遷移は起こりません。
最後のstateに到達したときの遷移先stateは一番初めのstateになります。
resourceパラメータを定義しないことで、ノードが描画されなくなりますが、そのノード以下をstateを使って操作することは可能です。
resourceにて記述するモデルとテクスチャのパスには、実行ディレクトリからの相対パスを記述してください。

カメラの設定については、一番最後に記述されたパラメータの値が扱われます。

## 参考

Vulkan Tutorial (<https://vulkan-tutorial.com/>)
Advanced vulkan rendering tutorial | multiple models | push constants | Part 3 (<https://www.youtube.com/watch?v=8AXTNMMWBGg>)
