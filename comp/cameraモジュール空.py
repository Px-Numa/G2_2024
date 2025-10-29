#########################################
#20241011
#Ito Natsuki
#カメラクラス
#モジュールは空
#########################################

import cv2
import time
import numpy as np


class Camera:
    def __init__(self):
        self.output_oshidashi_image_filepath = "output_oshidashi_image.jpg"
        self.output_fukuro_image_filepath = "output_fukuro_image.jpg"
        self.output_label_image_filepath = "output_label_image.jpg"
        self.output_kensa_image_filepath_upleft = "output_kensa_image_upleft.jpg"
        self.output_kensa_image_filepath_upright = "output_kensa_image_upright.jpg"
        self.output_kensa_image_filepath_downleft = "output_kensa_image_downleft.jpg"
        self.output_kensa_image_filepath_downright = "output_kensa_image_downright.jpg"
        
        self.measurement_time = 0
        self.measurement_start = 0
        self.measurement_end = 0
        
    def manage_camera(self):
        print("manage_cameraモジュールが動作しました")
        
    def detect_hand(self):
        print("detect_handモジュールが動作しました")
        
    def manage_image_processing(self):
        print("manage_image_processingモジュールが動作しました")
