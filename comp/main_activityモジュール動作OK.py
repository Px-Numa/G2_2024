##############################################
#2024/10/11
#Ito Natsuki
#メインクラス
#スレッド動作OK
##############################################

import os
os.environ["OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS"] = "0"

import fx3u
import camera
import socket
import threading
import signal
import time

class Main_activity:
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
        print("communicate_plcモジュールが動作しました")
        time.sleep(10)
        self.is_main_finish = True
        
            
        
        
        



if __name__ == '__main__':
    fx3u = fx3u.Fx3u(socket.gethostbyname(socket.gethostname()), 50000, 4096)
    camera = camera.Camera()
    main_activity = Main_activity()
    main_activity.Run() 