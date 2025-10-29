##############################################
#2024/10/11
#Ito Natsuki
#メインクラス
#fx3uクラス動作確認OK
##############################################

import os
os.environ["OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS"] = "0"

import fx3u
import camera
import socket
import threading
import signal
import time

import requests
import datetime

class MainActivity:
    ##初期設定
    def __init__(self):
        self.datetime_now = datetime.datetime.now()
        self.datetime_now = self.datetime_now.isoformat()
        
        self.url = "http://10.100.54.151/ito_test/log.php?q="
        
        self.is_main_finish = False
        self.exit_signal = False
        ##PLC通信スレッド
        self.thread_plc = threading.Thread(target=self.communicate_plc)
        ##カメラ管理スレッド
        self.thread_camera = threading.Thread(target=camera.manage_camera)
        ##手や身体の検出スレッド
        self.thread_hand = threading.Thread(target=camera.detect_hand)
        ##画像処理スレッド
        self.thread_image_processing = threading.Thread(target=camera.manage_image_processing)
        
        signal.signal(signal.SIGINT, self.Handler)
        
        self.thread_plc.setDaemon(True)
        self.thread_camera.setDaemon(True)
        self.thread_hand.setDaemon(True)
        self.thread_image_processing.setDaemon(True)
        
    def Run(self):
        ##各スレッドスタート
        self.thread_plc.start()
        self.thread_camera.start()
        self.thread_hand.start()
        self.thread_image_processing.start()
        
        while not self.is_main_finish and not camera.is_error_finish:
            time.sleep(0.1)
            
    def Handler(self, signal, frame):
        self.exit_signal = True
        
    def communicate_plc(self):
        fx3u.write_bitdevice('M20', 0)
        fx3u.write_bitdevice('M21', 0)
        fx3u.write_bitdevice('M22', 0)
        fx3u.write_bitdevice('M23', 0)
        fx3u.write_worddevice('D20', 1, 0)
        fx3u.write_worddevice('D21', 1, 0)
        fx3u.write_worddevice('D22', 1, 0)
        fx3u.write_worddevice('D93', 1, 0)
        os.system('cls')
        print("communicate_plcモジュールが動作しました")
        
        state = 1
        
        while(1):
            try:
                start = time.time()
                time.sleep(0.1)
        
                message_m = fx3u.read_bitdevice('M20')
                message_d = fx3u.read_worddevice('D91', 8)
                
                if message_d != '810000000000000000000000000000000000' and state == 1:
                    print("エラーが起こりました")
                    if message_d[4:8] != '0000':
                        print("非常停止")
                    if message_d[8:12] != '0000':
                        print("モニタ")
                    if message_d[12:16] != '0000':
                        print("人体検知")
                    if message_d[16:20] != '0000':
                        print("圧力不足")
                    if message_d[20:24] != '0000':
                        print("袋なし")
                    if message_d[24:28] != '0000':
                        print("パソコンの異常")
                    if message_d[28:32] != '0000':
                        print("異常な入力の組み合わせ")
                    if message_d[32:36] != '0000':
                        print("バッテリ残量低下")
                    state = 2
                    
                if state == 2:
                    if fx3u.read_bitdevice('M24') == '80001000':
                        print("エラーが解除されました")
                        state = 1
                        fx3u.write_bitdevice('M24', 0)
                
                
                #print(message_m)
                if message_m[4] == '1':
                    #print("ワークの有無判定")
                    print(fx3u.write_bitdevice('M20', 0))
                    camera.is_start_oshidashi = True
                
                if message_m[5] == '1':
                    #print("袋の有無判定")
                    print(fx3u.write_bitdevice('M21', 0))
                    camera.is_start_fukuro = True
                
                if message_m[6] == '1':
                    #print("破れの有無判定")
                    print(fx3u.write_bitdevice('M22', 0))
                    camera.is_start_kensa = True
                    
                if camera.is_send_oshidashi_result:
                    #print("-OK-ワークが正しく置かれています")
                    print(fx3u.write_worddevice('D20', 1, camera.oshidashi_result))
                    camera.is_send_oshidashi_result = False
                    
                if camera.is_send_fukuro_result:
                    #print("-OK-袋が置かれています")
                    print(fx3u.write_worddevice('D21', 1, camera.fukuro_result))
                    camera.is_send_fukuro_result = False
                    
                if camera.is_send_kensa_result:
                    #print("-OK-破れがありません")
                    print(fx3u.write_worddevice('D22', 1, camera.kensa_result))
                    camera.is_send_kensa_result = False
                
                if camera.is_detect_hand and camera.is_start_detect_hand:
                    #print("手が検出")
                    fx3u.write_worddevice('D93', 1, 2)
                else:
                    fx3u.write_worddevice('D93', 1, 1)
                    
                if message_m[7] == '1':
                    print("プログラムを終了します")
                    self.is_main_finish = True
                stop = time.time()
                #print(stop - start)
                
            except:
                self.datetime_now = datetime.datetime.now()
                self.datetime_now = self.datetime_now.isoformat()
                
                error_message = ":[Error] main_activity, communication error with PLC"

                response = requests.get(self.url + self.datetime_now + error_message)
                print (response.elapsed.total_seconds())
                import traceback
                traceback.print_exc()
                self.is_main_finish = True
               



if __name__ == '__main__':
    fx3u = fx3u.Fx3u(socket.gethostbyname(socket.gethostname()), 50000, 4096)
    camera = camera.Camera()
    main_activity = MainActivity()
    main_activity.Run() 