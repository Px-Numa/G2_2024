#------------------------------------------------------------------------
#date:2025/02/17
#name:Ito Natsuki
#取得画像から袋の破れを検出
#
#update:袋の反射を抑えるプログラムを追加
#------------------------------------------------------------------------



import cv2
import time
import numpy as np
from matplotlib import pyplot as plt

import os
os.environ["OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS"] = "0"

def nothing(x):
    pass



##window作成
#cv2.namedWindow('ori', cv2.WINDOW_NORMAL) #元画像
cv2.namedWindow('track', cv2.WINDOW_NORMAL)#トラックバー
#cv2.namedWindow('nitika_image', cv2.WINDOW_NORMAL)#２値化変更後画像
#cv2.namedWindow('contimg', cv2.WINDOW_NORMAL)#輪郭描画後画像
#cv2.namedWindow('open', cv2.WINDOW_NORMAL)#処理後画像

cv2.createTrackbar('canny1','track',94,255,nothing)#canny下
cv2.createTrackbar('canny2','track',23,255,nothing)#canny上
cv2.createTrackbar('heikatu','track',3,4,nothing)#平滑カーネル
cv2.createTrackbar('open','track',1,4,nothing)#オープニングカーネル
cv2.createTrackbar('open_count','track',3,10,nothing)#オープニング回数
cv2.createTrackbar('close_count','track',6,10,nothing)#クロージング回数


##表示させる画像のサイズを指定
##(幅,高さ)
size = (640,480)
#size = (1600, 1200)
#size = (3264, 2448)

while(1):
        ##ファイルパス
        filepath = "C:/startfile/g52024/image/out_img2.jpg"
        mask_filepath = "C:/startfile/g52024/image/oshidashi_staticimage.jpg"
        track_filepath = "C:/startfile/g52024/image/track.png"
        hansya_filepath = "C:/startfile/g52024/image/hansya_mask.jpg"
        
        ##画像読み込み
        frame = cv2.imread(filepath, 1)#カラー画像
        mask = cv2.imread(mask_filepath, 0)#袋の角マスク画像
        track = cv2.imread(track_filepath)#トラックバー
        hansya = cv2.imread(hansya_filepath, 0)#反射除去用のグレー画像
        image_color = cv2.imread(filepath,1)#カメラのカラー画像
        image_ori = cv2.imread(filepath)#カメラのグレー画像
        
        ##画像サイズ変更
        frame = cv2.resize(frame, size)
        mask = cv2.resize(mask, size)
        hansya = cv2.resize(hansya, size)
        image_color = cv2.resize(image_color, size)
        image_ori = cv2.resize(image_ori,size)
        
        cv2.imshow('frame', frame)#カメラ画像を表示
        
        frame = cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)#グレー画像に変換
        thresh_value, nitika_image = cv2.threshold(frame, 180, 255, cv2.THRESH_BINARY)#袋の反射部分を取得
        hihanten = nitika_image#袋の反射部分が白の画像
        hanten = 255 - nitika_image#袋の反射部分が黒の画像
        nitika_image = cv2.bitwise_and(frame, hanten)#元画像に対して、袋の反射部分を切り取る
        hansya = cv2.bitwise_and(hansya, hihanten)#グレー画像を袋の反射に合わせて切り取る
        nitika_image = cv2.add(nitika_image, hansya)#反射部分をグレー画像に差し替え
        
        cv2.imshow('hansya', nitika_image)#反射をなくした画像を表示


        image = nitika_image
        

        
        heikatu = cv2.getTrackbarPos('heikatu','track')  #オープニングカーネル(1)
        if heikatu == 1:
            heikatu = 1
        elif heikatu == 2:
            heikatu = 3
        elif heikatu == 3:
            heikatu = 5
        elif heikatu == 4:
            heikatu = 7
        else:
            heikatu = 1
        
        #平滑化
        image = cv2.medianBlur(image, heikatu)
        image = cv2.medianBlur(image, heikatu)
        image = cv2.medianBlur(image, heikatu)
        
        
        
        image_heikatu= image#平滑化後の画像
        
        ##キャニー処理でエッジ検出
        canny1 = cv2.getTrackbarPos('canny1','track')#トラックバーの値を取得
        canny2 = cv2.getTrackbarPos('canny2','track')#トラックバーの値を取得
        nitika_image = cv2.Canny(image,canny1,canny2)#キャニー処理
        
        #マスク処理
        nitika_image = cv2.bitwise_and(nitika_image, mask)
        
        ##輪郭検出----------------------------------------------------------------------
        nitika_image_color = cv2.cvtColor(nitika_image,cv2.COLOR_GRAY2BGR)#グレー画像をカラー画像に変換
        #ret,thresh = cv2.threshold(nitika_image, 63, 255, cv2.THRESH_BINARY)#白黒のエリアを決定
        contours,hierarchy = cv2.findContours(nitika_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)#輪郭を検出
        contimg = cv2.drawContours(nitika_image_color, contours, -1, (0, 255, 0), 7)#輪郭を書き込み
        
        ##穴あきを検出-----------------------------------------------------------------------------
        contimg_gray = cv2.cvtColor(contimg, cv2.COLOR_BGR2GRAY)#輪郭画像をグレー画像に変換
        thresh_value, image = cv2.threshold(contimg_gray, 100, 255, cv2.THRESH_BINARY)#輪郭部分を白色に
        nlabel, image_label = cv2.connectedComponents(image)#ラベリング処理
        #ラベリングされた部分を描画
        for i in range(1, nlabel):
            image_dst = cv2.compare(image_label, i, cv2.CMP_EQ)#元画像とラベリング画像を比較
            x, y, w, h = cv2.boundingRect(image_dst)#矩形領域を算出
            #ある程度大きい矩形を穴あきと判定
            if h >= 20:
                cv2.rectangle(image_color, (x,y), (x+w, y+h), (0,0,240), 3)#穴あき部分を囲む
        
        #画像を表示
        cv2.imshow('ori', image_color)#元画像
        cv2.imshow('open', image_heikatu)
        cv2.imshow('nitika_image',nitika_image)#バイラテラルフィルタの適用
        cv2.imshow('track',track)
        cv2.imshow('contimg',contimg)

        # 'q'キーが押されたらループから抜ける
        if cv2.waitKey(1) & 0xFF == ord('q'):
            cv2.imwrite("./image/ori.jpg", image_color)
            cv2.imwrite("./image/syorigo.jpg", image_heikatu)
            time.sleep(1)
            cv2.imwrite("./image/nitika.jpg", nitika_image)
            cv2.imwrite("./image/contimg.jpg", contimg)

            break





cv2.destroyAllWindows()




