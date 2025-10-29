#########################################
#20241015
#Ito Natsuki
#カメラクラス
#カメラ撮影画像から押出判定OK
#########################################

import cv2
import time
import numpy as np

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
        
        self.measurement_time = 0
        self.measurement_start = 0
        self.measurement_end = 0
        
        self.is_detect_hand = False
        self.is_start_oshidashi = False
        self.is_start_fukuro = False
        self.is_start_kensa = False
        self.is_start_qr = False
        
        self.a = False
        
        self.is_work = False
        
        
        
        
    ##カメラ起動、画像撮影
    def manage_camera(self):
        print("manage_cameraモジュールが動作しました")
        print("押出カメラ動作中")
        cv2.namedWindow("押出部")
        cap_oshidashi = cv2.VideoCapture(0)
        cap_oshidashi.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        while(1):
            time.sleep(0.2)
            ret1, self.frame_oshidashi = cap_oshidashi.read()
            if self.is_start_oshidashi:
                cv2.imwrite(self.output_oshidashi_image_filepath, self.frame_oshidashi)
                self.a = True
            
            
    ##手や身体の検出
    def detect_hand(self):
        print("detect_handモジュールが動作しました")
        while(1):
            time.sleep(0.2)
            if random.random() <= 0.0001:
                self.is_detect_hand = True
            else:
                self.is_detect_hand = False
    
    ##画像処理の動作管理
    def manage_image_processing(self):
        print("manage_image_processingモジュールが動作しました")
        while(1):
            time.sleep(0.05)
            #print(time.gmtime())
            if self.a:
                #oshidashi_image = cv2.imread(self.output_oshidashi_image_filepath)
                #oshidashi_image = cv2.imread(self.frame_oshidashi)
                #self.is_work = self.judge_oshidashi(oshidashi_image)
                self.is_work = self.judge_oshidashi(self.frame_oshidashi)
                self.is_start_oshidashi = False
                self.a = False
                
            if self.is_start_fukuro:
                self.judge_fukuro()
                self.is_start_fukuro = False
                
            if self.is_start_kensa:
                self.judge_kensa()
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
    
        whiteAreaRatio = (whitePixels/image_size)*100*7875#[cm^2]
        blackAreaRatio = (blackPixels/image_size)*100*7875#[cm^2]
    
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
    def judge_kensa(self):
        print("検査部の袋の破れ有無判定")
        
    ##QRコード読み取り
    def read_qr(self):
        print("QRコード読取り")
   
