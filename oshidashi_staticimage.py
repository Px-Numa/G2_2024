#------------------------------------------------------------------------
#date:2024/12/20
#name:Ito Natsuki
#リアルタイムで面積を算出
#------------------------------------------------------------------------

import cv2
import numpy as np


def calc_black_whiteArea(bw_image):
    image_size = bw_image.size
    whitePixels = cv2.countNonZero(bw_image)
    blackPixels = bw_image.size - whitePixels
 
    whiteAreaRatio = (whitePixels/image_size)*100
    blackAreaRatio = (blackPixels/image_size)*100
 
    print("White Area : ", whiteAreaRatio)
    print("Black Area : ", blackAreaRatio)

    return whiteAreaRatio

def nothing(x):
    pass


cv2.namedWindow('track')#トラックバー
cv2.createTrackbar('canny1','track',200,255,nothing)#canny下
cv2.createTrackbar('canny2','track',255,255,nothing)#canny上

size = (640,480)

if __name__ == '__main__':
    
    while(1):
        canny1 = cv2.getTrackbarPos('canny1','track')  #スレッショルド下(6)
        canny2 = cv2.getTrackbarPos('canny2','track')  #スレッショルド下(28)
        
        
        filepath = "C:/startfile/g52024/image/fukuro_image2.jpg"
        frame = cv2.imread(filepath,1)
        
        frame = cv2.resize(frame,size)
        cv2.imshow('frame', frame)
        
        frame = cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
        
        ##画像の読み込み
        track = cv2.imread("C:/startfile/g52024/image/track.png")
        
        #mask = cv2.imread("C:/startfile/g52024/image/mask_fukuro.jpg", 0)
        #frame = cv2.bitwise_and(mask, frame)
        
        #frame = track

        
        thresh_value, nitika_image = cv2.threshold(frame, canny1, canny2, cv2.THRESH_BINARY)
        #nitika_image = cv2.Canny(frame,canny1,canny2)
        #hihanten = nitika_image
        #hanten = 255 - nitika_image
        
        #nitika_image = cv2.bitwise_and(frame, hanten)
        
        #a = cv2.imread("C:/startfile/g52024/image/hansya_mask.jpg", 0)
        #a = cv2.bitwise_and(a, hihanten)
        
        #nitika_image = cv2.add(nitika_image, a)
        
        #print("撮影")
        cv2.imshow('ori', nitika_image)
        
        # 'q'キーが押されたらループから抜ける
        if cv2.waitKey(100) & 0xFF == ord('q'):
            white_area = calc_black_whiteArea(nitika_image)
            if white_area >= 350000:
                work = 1
            cv2.imwrite("./image/hansya.jpg", nitika_image)
            break
    
        
    
    cv2.destroyAllWindows()