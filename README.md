# Wallpaper_Matrix
## 概要
Windows用壁紙アニメーションアプリ<br>
映画マトリックスのように黒背景に緑の文字が移動するアニメーションを壁紙に直接描画する
## 操作
- 起動
```bat
wmatrix
```
![wmatrix](https://github.com/devhx2/Wallpaper_Matrix/blob/main/img/wmatrix.gif)
- 落下スピード変更<br>
1 (min) ~ 9 (max) で変更可能<br>
初期値は 7
```bat
wmatrix /spd 1
```
![wmatrix_spd_1](https://github.com/devhx2/Wallpaper_Matrix/blob/main/img/wmatrix_spd_1.gif)
- 文字色変更<br>
16進数表記(RGB)で変更可能<br>
```bat
wmatrix /fgc 0000ff
```
![wmatrix_fgc_00f](https://github.com/devhx2/Wallpaper_Matrix/blob/main/img/wmatrix_fgc_00f.gif)
- 背景色変更<br>
16進数表記(RGB)で変更可能<br>
```bat
wmatrix /bgc 00ff00
```
- 文字をバイナリに変更<br>
フラグで変更<br>
```bat
wmatrix /bin
```
