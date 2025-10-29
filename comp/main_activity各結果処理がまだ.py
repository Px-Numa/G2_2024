##############################################
#2024/10/16
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

class MainActivity:
    ##初期設定
    def __init__(self):
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
        
        while not self.is_main_finish:
            time.sleep(1)
            
    def Handler(self, signal, frame):
        self.exit_signal = True
        
    def communicate_plc(self):
        try:
            print("communicate_plcモジュールが動作しました")
            
            print(fx3u.write_bitdevice('M20', 0))
            print(fx3u.write_bitdevice('M21', 0))
            print(fx3u.write_bitdevice('M22', 1))
            message_m = fx3u.read_bitdevice('M20')
            print(message_m)
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
                
            time.sleep(3)
            if camera.is_work:
                print("ワークが正しく置かれています")
                print(fx3u.write_worddevice('D100', 1, 1))
            else:
                print("ワークがだめ")
                print(fx3u.write_worddevice('D100', 1, 2))
            
            print(fx3u.read_bitdevice('M20'))
            print("----------------------")
            if camera.is_detect_hand:
                print("手が検出")
                print(fx3u.write_worddevice('D101', 1, 1))
            
            
        except:
            import traceback
            traceback.print_exc()
            
        finally:
            print("5秒後にプログラムを終了します")
            time.sleep(5)
            self.is_main_finish = True
        
        
        
        
        
        



if __name__ == '__main__':
    fx3u = fx3u.Fx3u(socket.gethostbyname(socket.gethostname()), 50000, 4096)
    camera = camera.Camera()
    main_activity = MainActivity()
    main_activity.Run() 