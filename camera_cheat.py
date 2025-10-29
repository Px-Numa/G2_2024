#########################################################################
#file_name:camera.py
#date:2024/12/18
#name:伊東
#file_content:カメラクラス
#
#update:2025/01/06
#name:伊東
#update_content:袋破れ検査モジュールの修正（フィルタ処理と閾値の変更）
#
#update:2025/03/03
#name:伊東
#update_content:カメラなし、ラベル貼りなし、エラー情報登録なし
#               検査結果すべてOKと判定するプログラム
#               カメラ接続やサーバー接続なしでPLCを自動運転したいときに使用する
#########################################################################

import cv2
import time
import numpy as np
import mediapipe as mp
from matplotlib import pyplot as plt
import datetime


class Camera:
    ##初期設定---------------------------------------------------------------------------------------------------------------
    def __init__(self):
        #画像のファイルパスを設定
        self.output_oshidashi_image_filepath = "C:/startfile/g52024/image/output_oshidashi_image.jpg"
        self.output_fukuro_image_filepath = "C:/startfile/g52024/image/output_fukuro_image.jpg"
        self.output_label_image_filepath = "C:/startfile/g52024/image/output_label_image.jpg"
        self.output_kensa_image_filepath_upleft = "C:/startfile/g52024/image/output_kensa_image_upleft.jpg"
        self.output_kensa_image_filepath_upright = "C:/startfile/g52024/image/output_kensa_image_upright.jpg"
        self.output_kensa_image_filepath_downleft = "C:/startfile/g52024/image/output_kensa_image_downleft.jpg"
        self.output_kensa_image_filepath_downright = "C:/startfile/g52024/image/output_kensa_image_downright.jpg"
        
        self.file_path_nitika = "C:/startfile/g52024/image/nitika.jpg"
        self.file_path_rinkaku = "C:/startfile/g52024/image/rinkaku.jpg"
        self.file_path_result = "C:/startfile/g52024/image/result.jpg"
        
        self.mask_oshidashi_filepath = "C:/startfile/g52024/image/mask_oshidashi.jpg"
        
        #mediapipeの設定
        self.mp_drawing = mp.solutions.drawing_utils
        self.mp_drawing_styles = mp.solutions.drawing_styles
        self.mp_hands = mp.solutions.hands
        
        #各検査モジュールの開始信号
        self.is_start_oshidashi = False#押出部
        self.is_start_fukuro = False#袋セット部
        self.is_start_kensa = False#検査部
        self.is_start_qr = False#QR読取り部
        self.is_start_detect_hand = False#手検出部
        
        #各検査モジュールの検査終了信号
        self.is_send_oshidashi_result = False#押出部
        self.is_send_fukuro_result = False#袋セット部
        self.is_send_kensa_result = False#検査部
        self.is_detect_hand = False#手検出部
        self.is_ser_finish = False#ラベル貼り終了信号
        
        #各検査モジュールの検査結果
        self.oshidashi_result = 0#押出部
        self.fukuro_result = 0#袋セット部
        self.kensa_result = 0#検査部
        
        #各モジュールの終了信号
        self.is_manage_camera_finish = False

        #シャットダウン時の終了信号
        self.is_camera_shutdown = False#カメラクラス全体終了信号
        self.is_manage_camera_shutdown = False
        self.is_detect_hand_shutdown = False
        self.is_manage_image_processing = False

        self.datetime_now = datetime.datetime.now()#現在日時を取得
        self.datetime_now = self.datetime_now.isoformat()#日付データを文字列に変換
        
        self.url = "http://192.168.1.2/ito_test/log.php?q="#エラーログ書込みプログラムのURL（サーバー側）

        self.is_start_ser = True

        #押出カメラ表示ウインドウの終了信号
        self.finish_cap_oshidashi = False
        
        
        
            
    ##カメラ起動、画像撮影-------------------------------------------------------------------------------------------------------
    def manage_camera(self):
        print("manage_cameraモジュール（カメラ管理モジュール）が動作しました")
        self.frame_oshidashi = cv2.imread("image/oshidashi_image.jpg", 1)#カメラの代わりにあらかじめ画像を用意
        while(1):
            pass
        
    ##手や身体の検出-------------------------------------------------------------------------------------------------------------
    def detect_hand(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(3)
        print("detect_handモジュール（手検出モジュール）が動作しました")
        self.is_detect_hand = False
        with self.mp_hands.Hands(static_image_mode=True,
            max_num_hands=2, # 検出する手の数（最大2まで）
            min_detection_confidence=0.1) as hands:
            while(1):
                self.is_detect_hand = False
                pass
    
    
    ##画像処理の動作管理-------------------------------------------------------------------------------------------------------
    def manage_image_processing(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(3)
        print("manage_image_processingモジュール（画像処理動作モジュール）が動作しました")
        while(1):
            pass
    
    ##押出部のワーク有無判定------------------------------------------------------------------------------------------------------
    def judge_oshidashi(self, image):
        print("押出部のワーク有無判定")
        return
        
    ##袋セット部の袋の有無判定---------------------------------------------------------------------------------------------------
    def judge_fukuro(self, image):
        print("袋セット部の袋の有無判定")
        return
        
        
    ##検査部の袋の破れ有無判定----------------------------------------------------------------------------------------------------
    def judge_kensa(self, image, path):
        print("検査部の袋の破れ有無判定")
        return
        
    ##QRコード読み取り
    def read_qr(self, image):
        print("QRを読み取ります")
        return
   
