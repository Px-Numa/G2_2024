#########################################
#20241015
#Ito Natsuki
#カメラクラス
#カメラ撮影画像から押出判定OK
#########################################

import cv2
import time
import numpy as np
import mediapipe as mp

import random


class Camera:
    def __init__(self):
        self.output_oshidashi_image_filepath = "image/output_oshidashi_image.jpg"
        self.output_fukuro_image_filepath = "image/output_fukuro_image.jpg"
        self.output_label_image_filepath = "image/output_label_image.jpg"
        self.output_kensa_image_filepath_upleft = "image/output_kensa_image_upleft.jpg"
        self.output_kensa_image_filepath_upright = "image/output_kensa_image_upright.jpg"
        self.output_kensa_image_filepath_downleft = "image/output_kensa_image_downleft.jpg"
        self.output_kensa_image_filepath_downright = "image/output_kensa_image_downright.jpg"
        
        self.file_path_nitika = "./image/nitika.jpg"
        self.file_path_rinkaku = "./image/rinkaku.jpg"
        self.file_path_result = "./image/result.jpg"
        
        self.mp_drawing = mp.solutions.drawing_utils
        self.mp_drawing_styles = mp.solutions.drawing_styles
        self.mp_hands = mp.solutions.hands
        
        self.is_start_oshidashi = False
        self.is_start_fukuro = False
        self.is_start_kensa = False
        self.is_start_qr = False
        
        self.is_work = False
        self.is_detect_hand = False
        
        self.cap_oshidashi = cv2.VideoCapture(0)
        self.cap_oshidashi.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        
            
    ##カメラ起動、画像撮影
    def manage_camera(self):
        print("manage_cameraモジュールが動作しました")
        print("押出カメラ動作中")
        
        while(1):
            time.sleep(0.2)
            ret1, self.frame_oshidashi = self.cap_oshidashi.read()
            
            
    ##手や身体の検出
    def detect_hand(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(2)
        print("detect_handモジュールが動作しました")
        with self.mp_hands.Hands(static_image_mode=True,
            max_num_hands=2, # 検出する手の数（最大2まで）
            min_detection_confidence=0.5) as hands:
            while(1):
                try:
                    start = time.time()
                    time.sleep(0.1)
                    image = cv2.cvtColor(self.frame_oshidashi, cv2.COLOR_BGR2RGB)
                    image.flags.writeable = False

                    # 推論処理
                    hands_results = hands.process(image)

                    # 前処理の変換を戻しておく。
                    image.flags.writeable = True

                    # 有効なランドマーク（今回で言えば手）が検出された場合、
                    # ランドマークを描画します。
                    if hands_results.multi_hand_landmarks:
                        self.is_detect_hand = True
                        #print("手があります")
                    else:
                        self.is_detect_hand = False
                        #print("手がない")
                        
                    #stop = time.time()
                    #print(stop - start)
                    
                except:
                    self.is_detect_hand = True
                    print("予期せぬエラー（手判定）")
    
    
    ##画像処理の動作管理
    def manage_image_processing(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(2)
        print("manage_image_processingモジュールが動作しました")
        while(1):
            time.sleep(0.05)
            #print(time.gmtime())
            if self.is_start_oshidashi:
                self.is_work = self.judge_oshidashi(self.frame_oshidashi)
                self.is_start_oshidashi = False
                
            if self.is_start_fukuro:
                self.judge_fukuro()
                self.is_start_fukuro = False
                
            if self.is_start_kensa:
                self.judge_kensa(self.frame_oshidashi, self.output_kensa_image_filepath_upleft)
                self.is_start_kensa = False
                
            if self.is_start_qr:
                self.read_qr()
                self.is_start_qr = False
    
    
    ##押出部のワーク有無判定
    def judge_oshidashi(self, image):
        print("押出部のワーク有無判定")
        image_size = image.shape[0] * image.shape[1]
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        thresh_value, image_area = cv2.threshold(image, 200, 255, cv2.THRESH_BINARY)
        
        
        whitePixels = cv2.countNonZero(image_area)
        blackPixels = image_size - whitePixels
    
        whiteAreaRatio = (whitePixels/image_size) * 100 * 7875#[cm^2]
        blackAreaRatio = (blackPixels/image_size) * 100 * 7875#[cm^2]
    
        print("White Area [cm2] : ", whiteAreaRatio)
        print("Black Area [cm2] : ", blackAreaRatio)
        
        #cv2.imwrite("study/area.jpg", image_area)
        if whiteAreaRatio >= 300000:
            return True
        else:
            return False
        
    ##袋セット部の袋の有無判定
    def judge_fukuro(self):
        print("袋セット部の袋の有無判定")
        
        
    ##検査部の袋の破れ有無判定
    def judge_kensa(self, image, path):
        print("検査部の袋の破れ有無判定")
        self.measurement_start = time.time()
        
        cv2.imwrite(path, image)
        ##画像の読み込み
        self.image = cv2.imread(path,0)
        
        ##表示させる画像のサイズを指定
        ##(幅,高さ)
        size = (3264,2448)
        self.image = cv2.resize(self.image,size)


        ##window作成
        #cv2.namedWindow('ori') #元画像

        ##平滑化------------------------------------------
        #image = 255 - image   #ネガポジ
        self.image = cv2.blur(self.image, (3, 3))
        #cv2.imshow("img", image)
        #cv2.waitKey(0)

        #解説3
        # 構造要素は、モーフォロジー変換に使用されるカーネルで、ここでは5x5の正方形を使用します。
        kernel = np.ones((1,1),np.uint8)
        # 収縮、膨張、オープニング（収縮後の膨張）、クロージング（膨張後の収縮）を適用します。
        self.image = cv2.erode(self.image,kernel,iterations = 1)
        self.image = cv2.dilate(self.image,kernel,iterations = 1)
        self.image = cv2.erode(self.image,kernel,iterations = 1)
        self.image = cv2.dilate(self.image,kernel,iterations = 1)

        for i in range(100):
            self.image = cv2.morphologyEx(self.image, cv2.MORPH_OPEN, kernel)
            #image = cv2.morphologyEx(image, cv2.MORPH_CLOSE, kernel)


        ##2値化してエッジ処理
        th1 = 3
        th2 = 90
        nitika_image = cv2.Canny(self.image,th1,th2) 
        cv2.imwrite(self.file_path_nitika, nitika_image)
        
        ##角の部分だけを抽出
        nitika_image = cv2.imread(self.file_path_nitika, 1)
        mask = cv2.imread("image/mask2.jpg")
        nitika_image = cv2.bitwise_and(nitika_image, mask)

        ##画像を表示
        #cv2.imshow('ori', image)#元画像
        #cv2.imshow('nitika_image',nitika_image)#2値化後画像
        cv2.imwrite(self.file_path_nitika, nitika_image)

        #cv2.waitKey(0)




        #カラー画像を読み込んでグレースケール値に変換。
        img=cv2.imread(self.file_path_nitika)
        gray=cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

        #threshholdを使って白黒のエリアを決定する。
        ret,thresh = cv2.threshold(gray,63,255,cv2.THRESH_BINARY)
        #cv2.imshow('thresh',thresh)

        #輪郭を検出。
        contours,hierarchy = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        #輪郭を書き込み。
        contimg=cv2.drawContours(img,contours,-1,(0,255,0),3)

        #最終表示
        #cv2.imshow('final',contimg)
        cv2.imwrite(self.file_path_rinkaku, contimg)
        #cv2.waitKey(0)


        image_color = cv2.imread(path, 1)
        image_rinkaku = cv2.imread(self.file_path_rinkaku, 0)
        thresh_value, image_rinkaku = cv2.threshold(image_rinkaku, 100, 255, cv2.THRESH_BINARY)


        ##表示させる画像のサイズを指定
        ##(幅,高さ)
        image_rinkaku = cv2.resize(image_rinkaku,size)
        image_color = cv2.resize(image_color,size)



        nlabel, image_label = cv2.connectedComponents(image_rinkaku)

        is_yabure = False
        
        for i in range(1, nlabel):
            image_dst = cv2.compare(image_label, i, cv2.CMP_EQ)

            x, y, w, h = cv2.boundingRect(image_dst)
            aspectratio = float(h) / w
            print(aspectratio)
            #cv2.rectangle(image, (x,y), (x+w, y+h), 128)
            if aspectratio <= 1.5:
            #if aspectratio <= 1.1 and aspectratio >= 0.9:
            #if aspectratio == 0.9393939393939394:
                cv2.rectangle(image_color, (x,y), (x+w, y+h), (0, 255, 0))
                is_yabure = True

        #画像を表示
        #cv2.imshow('ori', image_color)#元画像

        #cv2.waitKey(0)
        cv2.imwrite(path, image_color)
        
        self.measurement_end = time.time()
        self.measurement_time = self.measurement_end - self.measurement_start
        
        print("計測時間：")
        print(self.measurement_time)
        
        return is_yabure
        
        
    ##QRコード読み取り
    def read_qr(self):
        print("QRコード読取り")
   
