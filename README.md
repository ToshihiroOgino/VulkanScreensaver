# Vulkan Screensaver

## 環境

Ubuntu 22.04上で開発やテストなどを行っています。その他の環境での動作は確認できていません。

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

シェダーは`glslc`を利用してコンパイルしてください。リポジトリのルートディレクトリにあるシェルスクリプトを利用することで、shadersフォルダ内のシェダーをコンパイルすることができます。

```bash
./CompileShaders.sh
```

以下のコマンドから、コードのコンパイルと実行を行えます。
`ARG=ファイル名`を追加することで、任意のスクリプトを実行できます。

```bash
make
# SRCを指定しない場合は"scripts/sample"を実行する
make run
# "scripts/complex"を実行する
make run ARG=scripts/complex
```

## スクリーンセーバー記述言語仕様

```plaintext
<code> ::= (<obj>)*
<obj> ::= <node> | <camera>

<node> ::= 'node{' ( <nodeParam> | ('state{' <stateParam>* '}') | <obj>)* '}'
<nodeParam> ::= (('time=' <number> ';' ) |
                ('resource=' <resourcePathSet> ) |
                ('position' | 'rotation' | 'scale') '=' <vector3> )';'
<stateParam> ::= (('time=' <number>) |
                (('position' | 'rotation' | 'scale') '=' <vector3> ))';'

<camera> ::= 'camera{' (<cameraParam>)* '}'
<cameraParam> ::= ( ('position' | 'lookAt' | 'angularVelocity') '=' <vector3>) | ( 'fov=' <number> ) )';'

<number> ::= ('-')? (0-9)+ ('.' (0-9)+)?
<vector3> ::= '(' <number> ',' <number> ',' <number> ')'
<resourcePathSet> ::= '( "' ((A-z)|'/'|'_'|'.')+  '" , "' ((A-z)|'/'|'_'|'.')+ '")'
```

- 宣言された`obj`中で記述されていない変数は既定の初期値が使用されます。
- `state`は`node`の動きを定義します。
- `state`は記述されているノード以下全てのノードの状態に影響します。
- `time`秒間かけて、次の`state`に遷移します。
- 最後の`state`の遷移先は最初の`state`です。
- `resource`パラメータを定義しないことで、ノードが描画されなくなりますが、`state`を使って子ノードを操作することは可能です。
- `resource`にて記述するモデルとテクスチャは、実行ディレクトリからの相対パスを利用します。
- `camera`の設定や`node`、`state`に同一パラメータが複数定義されている場合には、最後に記述されたパラメータの値が扱われます。
- 角度は度数法を用いて記述します。
- 数値は符号付き32ビット浮動小数点数に変換されます。
- 入力コード中に含まれる、空白や改行、その他の取扱いが未定義の文字は全て無視されます。
- `scale`が負になると面の法線の向きが反転します。

## パラメータ

### node

- resorce (文字列, 文字列) : モデルと画像のパスを指定する。
- state : ノードの状態を定義する
  - time (数値) : 次の状態へ遷移するのにかかる秒数
  - position (3次元ベクトル) : ノードの座標
  - rotation (3次元ベクトル) : ノードの回転 (度数法)
  - scale (3次元ベクトル) : ノードの大きさ

### camera

- position (3次元ベクトル) : カメラの座標
- lookAt (3次元ベクトル) : カメラが向いている座標
- angularVelocity (3次元ベクトル) : カメラの回転速度 (度/秒, 度数法)
- fov (数値) : カメラの垂直視野角 (度数法)

## 記述例

- Node1 (周囲のチェック柄の壁、不動)
- Node2 (VikingRoom、振り子のように動く)
  - Node3 (チェック柄の立方体、Node1と同じ動きをする)
- カメラの設定 (初期位置はx=3,y=3,z=3、原点を見ながら、25度/秒で原点を中心に回転する)


```
node{
    state{ scale=(0.5,0.5,0.5); }
    resource=("textures/checkerboard.jpg","models/cube_room.obj");
}
node{
    resource=("textures/viking_room.png","models/viking_room.obj");
    state{
        time=1;
        position=(-1,0,0);
        rotation=(0,45,90);
    }
    state{
        time=1;
        position=(1,0,0);
        rotation=(0,-45,90);
    }
    node{
        resource=("textures/checkerboard.jpg","models/cube.obj");
        state{
            position=(0,0,1);
            scale=(0.1,0.1,0.1);
        }
    }
}
camera{
    position=(3, 3, 3);
    lookAt=(0,0,0);
    angularVelocity=(0,0,25);
}
```

## 依存リポジトリ

座標計算 : [g-truc/glm](https://github.com/g-truc/glm.git)

画像読込 : [nothings/stb](https://github.com/nothings/stb.git)

3Dモデル読込 : [tinyobjloader/tinyobjloader](https://github.com/tinyobjloader/tinyobjloader.git)

描画 : [glfw/glfw](https://github.com/glfw/glfw.git)

描画 : [KhronosGroup/Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)

## 参考

[Vulkan Tutorial](https://vulkan-tutorial.com/)

[Advanced vulkan rendering tutorial | multiple models | push constants | Part 3](https://www.youtube.com/watch?v=8AXTNMMWBGg)
