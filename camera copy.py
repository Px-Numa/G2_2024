#########################################################################
#file_name:camera.py
#date:2024/12/18
#name:伊東
#file_content:カメラクラス
#
#update:2025/01/06
#name:伊東
#update_content:袋破れ検査モジュールの修正（フィルタ処理と閾値の変更）
#########################################################################

import cv2
import time
import numpy as np
import mediapipe as mp
from matplotlib import pyplot as plt

import random
import requests
import datetime
import serial


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
        
        #カメラ初期化
        #self.cap_oshidashi = cv2.VideoCapture(3)#押出部カメラ
        #self.cap_fukuro = cv2.VideoCapture(1)#袋セット部カメラ
        #self.cap_qr = cv2.VideoCapture(2)#qr部カメラ
        #self.cap_kensa1 = cv2.VideoCapture(3)#検査部カメラ左上
        #self.cap_kensa2 = cv2.VideoCapture(4)#検査部カメラ右上
        #self.cap_kensa3 = cv2.VideoCapture(5)#検査部カメラ左下
        #self.cap_kensa4 = cv2.VideoCapture(6)#検査部カメラ右下

        #self.cap_oshidashi.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)#押出部カメラのサイズ
        #self.cap_fukuro.set(cv2.CAP_PROP_FRAME_WIDTH, 640)#袋セット部カメラのサイズ


        #押出部のカメラ調整
        #self.cap_oshidashi.set(cv2.CAP_PROP_AUTOFOCUS, 0)#フォーカス自動調整
        #self.cap_oshidashi.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0)#露出自動調整
        #self.cap_oshidashi.set(cv2.CAP_PROP_EXPOSURE, -7)
        
        #袋セットのカメラ調整
        #self.cap_fukuro.set(cv2.CAP_PROP_AUTOFOCUS, 0)#フォーカス自動調整
        #self.cap_fukuro.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0)#露出自動調整
        #self.cap_fukuro.set(cv2.CAP_PROP_EXPOSURE, -8)
        """
        #QR読み取りのカメラ調整
        cap3.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1)#露出自動調整
        cap3.set(cv2.CAP_PROP_AUTOFOCUS, 1)#フォーカス自動調整
        """

        self.datetime_now = datetime.datetime.now()#現在日時を取得
        self.datetime_now = self.datetime_now.isoformat()#日付データを文字列に変換
        
        self.url = "http://192.168.1.2/ito_test/log.php?q="#エラーログ書込みプログラムのURL（サーバー側）

        self.is_start_ser = True

        #押出カメラ表示ウインドウの終了信号
        self.finish_cap_oshidashi = False
        
        
        
            
    ##カメラ起動、画像撮影-------------------------------------------------------------------------------------------------------
    def manage_camera(self):
        print("manage_cameraモジュール（カメラ管理モジュール）が動作しました")
        
        while(1):
            #time.sleep(0.1)
            #self.cap_oshidashi.set(cv2.CAP_PROP_EXPOSURE, -8)
            try:
                if self.is_camera_shutdown:
                    break
                self.frame_oshidashi = cv2.imread("image/oshidashi_image.jpg", 1)
                ##各カメラから画像を取得
                #ret1, self.frame_oshidashi = self.cap_oshidashi.read()
                #ret2, self.frame_fukuro = self.cap_fukuro.read()
                #ret3, self.frame_qr = self.cap_qr.read()
                #ret4, self.frame_kensa1 = self.cap_kensa1.read()
                #ret5, self.frame_kensa2 = self.cap_kensa2.read()
                #ret6, self.frame_kensa3 = self.cap_kensa3.read()
                #ret7, self.frame_kensa4 = self.cap_kensa4.read()
                #if ret1 == False:
                #    break
                
                #if(self.finish_cap_oshidashi):
                #    self.cap_oshidashi.release()
                    #self.cap_fukuro.release()
                    #self.cap_qr.release()
                    #self.cap_kensa1.release()
                    #self.cap_kensa2.release()
                    #self.cap_kensa3.release()
                    #self.cap_kensa4.release()
                #    cv2.destroyAllWindows()
                #    time.sleep(5)

                #cv2.imshow('oshidashi', self.frame_oshidashi)
                #cv2.imshow('fukuro', self.frame_fukuro)
                #cv2.imshow('qr', self.frame_qr)
                #cv2.imshow('左上', self.frame_kensa1)
                #cv2.imshow('右上', self.frame_kensa2)
                #cv2.imshow('左下', self.frame_kensa3)
                #cv2.imshow('右下', self.frame_kensa4)

                key = cv2.waitKey(1)
                
            #manage_cameraの例外処理
            except:
                self.datetime_now = datetime.datetime.now()#現在日時を取得
                self.datetime_now = self.datetime_now.isoformat()#日付データを文字列に変換
                #error_message = ":[Error] manage_camera, communication error with camera"#エラーメッセージを設定
                #response = requests.get(self.url + self.datetime_now + error_message)#エラーログ書込みプログラムを実行（GETで日付データとエラーメッセージを送信）
                #print (response.elapsed.total_seconds())
                import traceback
                traceback.print_exc()
                self.is_manage_camera_finish = True#画像撮影プログラム終了信号をTrueに
                print("画像撮影プログラム(while中の例外)でエラーが発生しました")
                time.sleep(5)

        print("manage_cameraモジュール（カメラ管理モジュール）を終了します")
        self.is_manage_camera_shutdown = True#シャットダウン信号をTrueに

        #カメラの接続不良が起こったら動作
        self.datetime_now = datetime.datetime.now()#現在日時を取得
        self.datetime_now = self.datetime_now.isoformat()#日付データを文字列に変換
        #error_message = ":[Error] manage_camera, communication error with camera"#エラーメッセージを設定
        #response = requests.get(self.url + self.datetime_now + error_message)#エラーログ書込みプログラムを実行（GETで日付データとエラーメッセージを送信）
        #print (response.elapsed.total_seconds())
        import traceback
        #traceback.print_exc()
        #self.is_manage_camera_finish = True#画像撮影プログラム終了信号をTrueに
        #print("画像撮影プログラムでエラーが発生しました")
        #time.sleep(5)
            
            
    ##手や身体の検出-------------------------------------------------------------------------------------------------------------
    def detect_hand(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(5)
        print("detect_handモジュール（手検出モジュール）が動作しました")
        self.is_detect_hand = False
        cv2.namedWindow('hands', cv2.WINDOW_NORMAL)
        mask = cv2.imread("image/work_jintai.jpg", 1)
        with self.mp_hands.Hands(static_image_mode=True,
            max_num_hands=2, # 検出する手の数（最大2まで）
            min_detection_confidence=0.1) as hands:
            while(1):
                self.is_detect_hand = False
                return
                #time.sleep(0.05)
                try:
                    if self.is_camera_shutdown:
                        break

                    image = cv2.cvtColor(self.frame_oshidashi, cv2.COLOR_BGR2RGB)
                    imagea = cv2.cvtColor(self.frame_oshidashi, cv2.COLOR_BGR2RGB)
                    image.flags.writeable = False

                    image = cv2.bitwise_and(image, mask)

                    # 推論処理
                    hands_results = hands.process(image)

                    # 前処理の変換を戻しておく。
                    image.flags.writeable = True
                    write_image = cv2.cvtColor(imagea, cv2.COLOR_RGB2BGR)

                    #手が検出された場合、
                    if hands_results.multi_hand_landmarks:
                        for landmarks in hands_results.multi_hand_landmarks:
                            self.mp_drawing.draw_landmarks(
                                write_image,
                                landmarks,
                                self.mp_hands.HAND_CONNECTIONS,
                                self.mp_drawing_styles.get_default_hand_landmarks_style(),
                                self.mp_drawing_styles.get_default_hand_connections_style())

                        self.is_detect_hand = True
                    else:
                        self.is_detect_hand = False
                    
                    """
                    self.mp_drawing = mp.solutions.drawing_utils
                    self.mp_drawing_styles = mp.solutions.drawing_styles
                    self.mp_hands = mp.solutions.hands
                    """

                    # ディスプレイ表示
                    #cv2.imshow('hands', write_image)

                    key = cv2.waitKey(1)


                except:
                    self.is_detect_hand = True
                    #print("予期せぬエラー（手判定）")

            print("detect_handモジュール（手検出モジュール）を終了します")
            self.is_detect_hand_shutdown = True#シャットダウン信号をTrueに
    
    
    ##画像処理の動作管理-------------------------------------------------------------------------------------------------------
    def manage_image_processing(self):
        ##カメラが最初の画像を取得するまで待機
        time.sleep(5)
        print("manage_image_processingモジュール（画像処理動作モジュール）が動作しました")
        #self.read_qr(self.frame_oshidashi)
        while(1):
            time.sleep(0.1)
            if self.is_camera_shutdown:
                break
            #押出部の開始信号がTrueになったら動作
            if self.is_start_oshidashi:
                self.judge_oshidashi(self.frame_oshidashi)#押出部のワーク有無判定モジュールの実行
                self.is_send_oshidashi_result = True#押出部終了信号をTrueに
                self.is_start_detect_hand = True#手や身体検出開始信号をTrueに
                self.is_start_oshidashi = False#押出部開始信号をFalseに
                #self.is_start_ser = True
                #self.read_qr(self.frame_oshidashi)
                #time.sleep(2)
            #袋セット部の開始信号がTrueになったら動作
            if self.is_start_fukuro:
                start = time.time()
                self.judge_fukuro(self.frame_oshidashi)#袋セット部の袋有無判定モジュールの実行
                self.is_send_fukuro_result = True#袋セット部終了信号をTrueに
                self.is_start_fukuro = False#袋セット部開始信号をFlaseに
                stop = time.time()
                print("袋セット部の検査にかかった時間は")
                print(stop - start)
                #time.sleep(2)
            #検査部の開始信号がTrueになったら動作
            if self.is_start_kensa:
                self.judge_kensa(self.frame_oshidashi, self.output_kensa_image_filepath_upright)#検査部の袋穴あき検査モジュールの実行
                #print(self.read_qr(self.frame_oshidashi))#qr読取りモジュールの実行
                #time.sleep(20)
                self.is_send_kensa_result = True#検査部終了信号をTrueに
                self.is_start_kensa = False#検査部開始信号をFalseに
                print("検査部を終了します")
                
                #time.sleep(10)
                
            if self.is_start_qr:
                self.read_qr()
                self.is_start_qr = False

        print("manage_image_processingモジュール（画像処理動作モジュール）を終了します")
        self.is_manage_image_processing = True#シャットダウン信号をTrueに
    
    ##押出部のワーク有無判定------------------------------------------------------------------------------------------------------
    def judge_oshidashi(self, image):
        print("押出部のワーク有無判定")
        
        size = (1920, 1080)#画像サイズ
        mask = cv2.imread(self.mask_oshidashi_filepath, 0)#押出部のマスク画像の読み込み
        mask = cv2.resize(mask,size)#マスク画像をリサイズ
        image_size = image.shape[0] * image.shape[1]#画像サイズの面積を算出
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)#画像をグレースケールに変換
        #image = cv2.bitwise_and(image, mask)#マスク処理
        thresh_value, image_area = cv2.threshold(image, 89, 255, cv2.THRESH_BINARY)#2値化処理
        whitePixels = cv2.countNonZero(image_area)#白色の部分を算出
        blackPixels = image_size - whitePixels#黒色の部分を算出
    
        whiteAreaRatio = (whitePixels/image_size) * 100#白色の部分の割合を算出
        blackAreaRatio = (blackPixels/image_size) * 100#黒色の部分の割合を算出
    
        print("White Area [cm2] : ", whiteAreaRatio)
        print("Black Area [cm2] : ", blackAreaRatio)
        
        cv2.imwrite("image/oshidashi_result.jpg", image_area)
        #if whiteAreaRatio >= 24.5:
        if whiteAreaRatio >= 24.45:
            self.oshidashi_result = 1#ワークセットOK
            print("ワークセットOK")
        else:
            self.oshidashi_result = 2#ワークセットNG
            print("ワークセットNG")
        
    ##袋セット部の袋の有無判定---------------------------------------------------------------------------------------------------
    def judge_fukuro(self, image):
        print("袋セット部の袋の有無判定")
        cv2.imwrite("image/fukuro_result.jpg", image)
        
        if image[10, 20][0] <= 150:
            self.fukuro_result = 1#袋あり
            print("袋あり")
        else:
            self.fukuro_result = 2#袋なし
            print("袋なし")
        
        
    ##検査部の袋の破れ有無判定----------------------------------------------------------------------------------------------------
    def judge_kensa(self, image, path):
        print("検査部の袋の破れ有無判定")
        measurement_start = time.time()
        image_color = image
        
        ##表示させる画像のサイズを指定
        ##(幅,高さ)
        size = (640,480)
        image = cv2.resize(image,size)
        mask = cv2.imread("C:/startfile/g52024/image/mask2.jpg", 0)
        mask = cv2.resize(mask,size)


        ##window作成
        #cv2.namedWindow('ori') #元画像

        ##平滑化------------------------------------------
        #image = 255 - image   #ネガポジ
        image = cv2.blur(image, (3, 3))
        #cv2.imshow("img", image)
        #cv2.waitKey(0)

        #解説3
        # 構造要素は、モーフォロジー変換に使用されるカーネルで、ここでは5x5の正方形を使用します。
        kernel = np.ones((3,3),np.uint8)
        # 収縮、膨張、オープニング（収縮後の膨張）、クロージング（膨張後の収縮）を適用します。
        for i in range(4):
            image = cv2.dilate(image,kernel,iterations = 1)
            
        for i in range(4):
            image = cv2.erode(image,kernel,iterations = 1)

        ##2値化してエッジ処理
        th1 = 150
        th2 = 200
        nitika_image = cv2.Canny(image,th1,th2) 
        
        
        nitika_image = cv2.bitwise_and(nitika_image, mask)

        ##画像を表示
        #cv2.imshow('ori', image)#元画像
        #cv2.imshow('nitika_image',nitika_image)#2値化後画像
        #cv2.imwrite(self.file_path_nitika, nitika_image)

        #cv2.waitKey(0)

        


        #カラー画像を読み込んでグレースケール値に変換。
        img=cv2.cvtColor(nitika_image, cv2.COLOR_GRAY2BGR)
        #gray=cv2.cvtColor(nitika_image, cv2.COLOR_BGR2GRAY)

        #threshholdを使って白黒のエリアを決定する。
        ret,thresh = cv2.threshold(nitika_image,63,255,cv2.THRESH_BINARY)
        #cv2.imshow('thresh',thresh)

        cv2.imwrite("image/a.jpg", thresh)

        #輪郭を検出。
        contours,hierarchy = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        #輪郭を書き込み。
        contimg=cv2.drawContours(img,contours,-1,(0,255,0),3)

        #image_color = cv2.imread(path, 1)
        image_rinkaku = cv2.cvtColor(contimg, cv2.COLOR_BGR2GRAY)
        thresh_value, image_rinkaku = cv2.threshold(image_rinkaku, 100, 255, cv2.THRESH_BINARY)

        cv2.imwrite("image/c.jpg", image_rinkaku)

        ##表示させる画像のサイズを指定
        ##(幅,高さ)
        image_rinkaku = cv2.resize(image_rinkaku,size)
        image_color = cv2.resize(image_color,size)

        nlabel, image_label = cv2.connectedComponents(image_rinkaku)

        self.kensa_result = 1#破れなし(1)
        for i in range(1, nlabel):
            image_dst = cv2.compare(image_label, i, cv2.CMP_EQ)

            x, y, w, h = cv2.boundingRect(image_dst)
            aspectratio = float(h) / w
            #print(aspectratio)
            #cv2.rectangle(image, (x,y), (x+w, y+h), 128)
            if aspectratio <= 100:
            #if aspectratio <= 1.1 and aspectratio >= 0.9:
            #if aspectratio == 0.9393939393939394:
                cv2.rectangle(image_color, (x,y), (x+w, y+h), (0, 255, 0))
                self.kensa_result = 2#破れあり(2)

        #画像を表示
        #cv2.imshow('ori', image_color)#元画像

        #cv2.waitKey(0)
        #cv2.imwrite(path, image_color)
        cv2.imwrite(path, image_color)
        
        measurement_end = time.time()
        measurement_time = measurement_end - measurement_start
        
        print("計測時間：")
        print(measurement_time)
        
        
    ##QRコード読み取り
    def read_qr(self, image):
        print("QRを読み取ります")
        #ser = serial.Serial("COM5", 9600) # ここのポート番号を変更
        #val = 100 # ここの値を変更すると、Arduinoの挙動が変わる
        #time.sleep(13)
        #return "1111, aaa, 111"
        #"""
        if self.is_start_ser:
            self.is_start_ser = False
            print("ラベル貼りと接続します")
            self.ser = serial.Serial("COM5", 9600)
            print("初期動作を終了します")
        elif self.is_ser_finish:
            pass
        else:
            print("動作開始")
            self.ser.write(bytes('1','utf-8'))
            print("送信完了")

            time.sleep(15)

            #while val_arduino != '2':
            #    try:
            #        val_arduino = self.ser.readline()
            #        print("2を受け取りました")
            #    except:
            #        print("例外")
                
            print("終了")
            self.is_ser_finish = True
            #self.ser.close()
        return

        # QRCodeDetectorオブジェクトを作成する
        detector = cv2.QRCodeDetector()
        print("QRコード読取り")
        try:
            retval, decoded_info, points, straight_qrcode = detector.detectAndDecodeMulti(image)
            # 検出したQRコードの座標のデータ型を"int"に変換
            points = points.astype(int)
            # 描画設定
            is_closed = True                    # QRコードを囲う四角形を閉包図形とする
            font = cv2.FONT_HERSHEY_SIMPLEX     # フォントの種類
            font_color = (255, 0, 255)          # フォントの色をマゼンタに設定
            font_line_type = cv2.LINE_AA        # フォントをアンチエイリアスで描画
            # 検出したQRコードのそれぞれを四角で囲い、デコードしたデータを描画する
            
            ##QRコードの文字列を表示
            print(decoded_info[0])
            ##区切り文字指定
            dem = decoded_info[0].split(',')

            ##区切り後の文字列を表示
            print(dem[0])
            print(dem[1])
            print(dem[2])
            
        
        except:
            import traceback
            traceback.print_exc()
   
