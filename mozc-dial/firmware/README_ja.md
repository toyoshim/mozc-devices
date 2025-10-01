# ファームウェア開発ガイド

キーボードの組み立て全体については[ビルドガイド](../buildguide_ja.md)を参照してください。

## ディレクトリ一覧

- prebuilt/ : 各ボード向けに事前にビルドしたファームウェア
- common/ : 各ボード共通で用いられるライブラリのソースファイル
- main/ : 9ダイヤル版メインチップ用のソースファイルとプロジェクト
- sub/ : 9ダイヤル版サブチップ用のソースファイルとプロジェクト
- one_dial/ : 1ダイヤル版 Raspberry Pi Pico用のソーとファイルとプロジェクト

## 事前にビルドしたファームウェア

`prebuilt/`以下に事前にビルドしたファームウェアを収めていますので、変更が不要の場合はこれらをそのまま用いることができます。

9ダイヤル板は`main.uf2`と`sub.uf2`を、1ダイヤル板は`one_dial.uf2`を使用します。

## 独自のファームウェア開発の手引き

[Visual Studio Code](https://code.visualstudio.com/) をインストールし、[Raspberry Pi Pico拡張](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) を追加します。

Activity Barの`Raspberry Pi Pico Projects`の拡張機能から`Import Project`を使い、各プロジェクトのディレクトリを開いてください。
拡張をはじめて使う場合、開発環境のインストールが始まります。プロジェクトが取り込まれると、Activity Barの`CMake`や`Run and Debug`を使って開発できるようになります。

公式のPicoシリーズボードに搭載されているように、9ダイヤル板でも基板上に`SWC/D`と書かれた`SWCLK`と`SWDIO`のランドペアがそれぞれのチップごとに用意されています。公式のDebug Probe等のSWDをサポートした開発機材を用意する事で、エディタ上でステップ実効等が可能です。

### 標準出力について

9ダイヤル板では基板上にメインチップ用の`TX1`、サブチップ用の`TX2`と書かれたランドが用意されており、標準の設定では標準出力はこれらのピンから速度115200のUARTとして出力されています。

一方で`CMakeLists.txt`の`pico_enable_stdio_uart`を`0`に、`pico_enable_stdio_semihosting`を`1`に変更する事で、出力をデバッガ経由に切り替える事ができます。TXを配線せずに済むので便利ですが、デバッガのサポートを有効にしないと出力のところで動作停止するので注意が必要です。デバッガがアタッチされていない時も同様に停止するので、リリースビルドには使えません。デバッガのサポートは`monitor arm semihosting enable`で有効に出来ますが、デバッガ起動ごとに毎回設定するのは大変なので、`.vscode/launch.json`の各コマンド設定内に以下のように登録しておくと便利です。

```
"postLaunchCommands": [
    "monitor arm semihosting enable"
]
```

### PICO WやPICO 2 (W)の利用

1ダイヤル板は公式のRaspberry Pi Picoボードを使うように設定してありますが、`CMakeFiles.txt`内の`set(PICO_BOARD pico CACHE STRING "Board type")`のところでボード設定を`pico`から`pico_w`、`pico2`、`pico2_w`等に変更する事で他のボードにも対応できるかと思います。3rd party製のボードについても`none`等で対応できるかと思います。

### EEPROMについて

9ダイヤル版はボード設定を`none`にしているので、EEPROMについては汎用のドライバが分周4で組み込まれます。これでだいたいのEEPROMで動作するはずです。リファレンスの部品を使っている場合は、公式ボードと同様にW25Q080用のドライバが分周2で動作する事を確認しています。`~/.pico-sdk/sdk/2.2.0/src/boards/include/boards/`に`ボード名.h`を作成し、その中で

```
#define PICO_BOOT_STAGE_2_CHOOSE_W25Q080 1
#define PICO_FLASH_SPI_CLKDIV 2
```

などを定義し、`CMakeFiles.txt`のボード設定で指定する事で、キャッシュミス時のQSPI経由でのEEPROMアクセスを最適化し、さらなる性能を引き出す事も可能です。

### センサーの調整について

標準ではセンサーは内部抵抗でpull-upされています。外部抵抗によりpull-upで調整したい場合、`photo_sensor.cc`の`PhotoSensor::PhotoSensor()`内にある`gpio_pull_up(gpio);`を`gpio_disable_pulls(gpio);`に変更してください。

あるいは、内部抵抗を有効にしたまま外部抵抗との合成抵抗でpull-upする事も可能です。この場合、内部抵抗は公称値で50-80KΩ、外部抵抗とは並列接続になります。

詳細は[基板組立ガイド](../board/README_ja.md)を確認してください。
