#########################################################################
#file_name:main_activity.py
#date:2024/12/18
#name:伊東
#file_content:メインクラス
#
#update:2025/01/06
#name:伊東
#update_content:エラーログ書込み処理の追加とコメント書込み
#
#update:2025/03/03
#name:伊東
#update_content:カメラなし、ラベル貼りなし、エラー情報登録なし
#               検査結果すべてOKと判定するプログラム
#               カメラ接続やサーバー接続なしでPLCを自動運転したいときに使用する
#               M155を1にするとプログラムが終了する
#########################################################################

import os
import fx3u
import camera_cheat
import socket
import threading
import signal
import time
import datetime

class MainActivity:
    ##初期設定
    def __init__(self):
        os.system('cls')#コマンドクリア
        self.is_main_finish = False#メインプログラム終了信号
        self.exit_signal = False
        
        #スレッド設定
        self.thread_plc = threading.Thread(target=self.communicate_plc)##PLC通信スレッド
        self.thread_camera = threading.Thread(target=camera.manage_camera)##カメラ管理スレッド
        self.thread_hand = threading.Thread(target=camera.detect_hand)##手や身体の検出スレッド
        self.thread_image_processing = threading.Thread(target=camera.manage_image_processing)##画像処理スレッド
        
        signal.signal(signal.SIGINT, self.Handler)
        
        #スレッドをデーモン化
        self.thread_plc.setDaemon(True)
        self.thread_camera.setDaemon(True)
        self.thread_hand.setDaemon(True)
        self.thread_image_processing.setDaemon(True)

        self.is_oshidashi_state = True
        self.is_fukuro_state = True
        self.is_kensa_state = True
        
        #実行モード
        path = "C:/startfile/g52024/finish.txt"
        with open(path, mode='w') as f:
            f.write("1")
        
    
    #各スレッドをスタートし、終了信号が検出されるまで無限ループ
    def Run(self):
        ##各スレッドスタート
        self.thread_plc.start()
        self.thread_camera.start()
        self.thread_hand.start()
        self.thread_image_processing.start()
        
        #各処理の終了信号がFalseの間連続動作
        while   not self.is_main_finish and  \
                not camera.is_manage_camera_finish:
            pass

        if camera.is_camera_shutdown:
            print("シャットダウンします")
            os.system('shutdown /s /t 1')
        else:
            print("プログラムを終了します")
            time.sleep(2)
        #os.system('shutdown /s /t 1')
            
    def Handler(self, signal, frame):
        self.exit_signal = True
    
    #PLC通信（メインプログラム）
    def communicate_plc(self):
        print(fx3u.write_bitdevice('M20', 0))
        print(fx3u.write_bitdevice('M21', 0))
        print(fx3u.write_bitdevice('M22', 0))
        print(fx3u.write_bitdevice('M155', 0))

        #現在日時をPLCに送信
        dt_now = datetime.datetime.now()
        print(fx3u.write_worddevice('D50', 1, dt_now.year))#年
        print(fx3u.write_worddevice('D51', 1, dt_now.month))#月
        print(fx3u.write_worddevice('D52', 1, dt_now.day))#日
        print(fx3u.write_worddevice('D53', 1, dt_now.hour))#時
        print(fx3u.write_worddevice('D54', 1, dt_now.minute))#分
        print(fx3u.write_worddevice('D55', 1, dt_now.second))#日
        time.sleep(1)
        print(fx3u.write_bitdevice('M55', 1))
        
        print("communicate_plcモジュール（PLC通信モジュール）が動作しました")
        
        error_state = 1
        path = 'C:/startfile/g52024/finish.txt'
        
        while(1):
            try:
                start = time.time()#whileの1ループタイム測定スタート
                with open(path) as f:
                    s = f.read()
                if s == '2':
                    print("シャットダウン信号が入りました")
                    break
                if s == '3':
                    print("プログラムを終了")
                    self.is_main_finish = True

                message_m = fx3u.read_bitdevice('M20')#M20～M23までの値を取得を取得(カメラ許可信号)
                message_finish = fx3u.read_bitdevice('M154')#M154の値を取得(装置終了信号)
                message_d = fx3u.read_worddevice('D91', 10)#D91～D100までの値を取得(エラー信号)
                
                #PLC側にエラーが出たとき動作
                if message_d != '81000000000000000000000000000000000000000000' and error_state == 1:
                    print(message_d)
                    print("エラー発生")
                    error_id = 0
                    if message_d[4:8] != '0000':
                        print("非常停止")
                        error_id = 1#非常停止
                    if message_d[8:12] != '0000':
                        error_id = 2#DC24Vモニタ
                        print("モニタ")
                    if message_d[12:16] != '0000':
                        error_id = 3#人体検知
                        print("人体検知")
                    if message_d[16:20] != '0000':
                        error_id = 4#一次空気圧力不足
                        print("圧力不足")
                    if message_d[20:24] != '0000':
                        error_id = 5#袋がない
                        print("袋なし")
                    if message_d[24:28] != '0000':
                        error_id = 6#パソコンの異常
                        print("パソコンの異常")
                    if message_d[28:32] != '0000':
                        error_id = 7#機器が異常
                        print("異常な入力の組み合わせ")
                    if message_d[32:36] != '0000':
                        error_id = 8
                        print("バッテリ残量低下")
                    if message_d[36:40] != '0000':
                        error_id = 9
                        print("稼働時間")
                    
                    error_state = 2#ステータスをエラー中に変更
                
                #PLC側のエラーが解決したときに動作
                if error_state == 2:
                    if message_d == '81000000000000000000000000000000000000000000':
                        print("エラーが解除されました")
                        
                        error_state = 1#ステータスをエラーなしに変更

                
                #押出部カメラ許可信号を受け取ったら動作
                if message_m[4] == '1':
                    fx3u.write_bitdevice('M20', 0)#カメラ許可信号をリセット
                    message_m[4] == '0'
                    print("押し出し部のカメラ許可が出ました")
                    camera.is_send_oshidashi_result = True
                #袋セット部カメラ許可信号を受け取ったら動作
                if message_m[5] == '1':
                    fx3u.write_bitdevice('M21', 0)#カメラ許可信号をリセット
                    message_m[5] == '0'
                    print("袋セット部のカメラ許可が出ました")
                    camera.is_send_fukuro_result = True
                #検査部カメラ許可信号を受け取ったら動作
                if message_m[6] == '1':
                    fx3u.write_bitdevice('M22', 0)#カメラ許可信号をリセット
                    message_m[6] == '0'
                    print("検査部のカメラ許可が出ました")
                    camera.is_send_kensa_result = True
                    

                    
                #押出部の検査が完了したら動作
                if camera.is_send_oshidashi_result:
                    print("押出部の検査が終了しました")
                    fx3u.write_worddevice('D20', 1, 1)#押出部の検査結果を書込み
                    camera.is_send_oshidashi_result = False#押出部の検査完了信号をリセット
                #袋セット部の検査が完了したら動作
                if camera.is_send_fukuro_result:
                    print("袋セット部の検査が終了しました")
                    fx3u.write_worddevice('D21', 1, 1)#袋セット部の検査結果を書込み
                    camera.is_send_fukuro_result = False#袋セット部の検査完了信号をリセット
                #検査部の検査が完了したら動作
                if camera.is_send_kensa_result:
                    print("検査部の検査が終了しました")
                    fx3u.write_worddevice('D22', 1, 1)#検査部の検査結果を書込み
                    camera.is_send_kensa_result = False#検査部の検査完了信号をリセット
                    
                       
                #押出部のワークセット完了後、装置内に手や身体を検出したら動作
                if camera.is_detect_hand:
                    fx3u.write_worddevice2('D23', 2, 1, 1)
                    print("手が検出されました")
                else:
                    #fx3u.write_worddevice('D23', 1, 1)
                    fx3u.write_worddevice2('D23', 2, 1, 1)
                
                #M155が1ならメインプログラム終了信号をTrueに
                if message_finish[5] == '1':
                    camera.finish_cap_oshidashi = True
                    self.is_main_finish = True
                    print("終了します")
                    #fx3u.finish_signal()
                    time.sleep(3)
                    print("正しく終了しました")
                    
                stop = time.time()#whileの1ループタイム測定終了
                #print(stop - start)#whileの1ループタイムを表示
            
            #PLC通信モジュール例外処理
            except:
                pass

        print("終了処理に入ります")
        camera.is_camera_shutdown = True
        print("全てのモジュールが終了しました")
        print("安全にシャットダウン処理に入ります")
        self.is_main_finish = True
               
if __name__ == '__main__':
    print("メインプログラムを開始します")
    time.sleep(1)

    #fx3u = fx3u.Fx3u(socket.gethostbyname(socket.gethostname()), 50000, 4096)#fx3u通信クラスをインスタンス化
    fx3u = fx3u.Fx3u("192.168.1.250", 5000, 4096)#fx3u通信クラスをインスタンス化
    camera = camera_cheat.Camera()#カメラ制御クラスをインスタンス化
    main_activity = MainActivity()#メインクラスをインスタンス化
    main_activity.Run() 