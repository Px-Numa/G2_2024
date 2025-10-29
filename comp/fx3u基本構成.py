##############################################
#2024/09/20
#Ito Natsuki
#fx3uシリーズ　コマンドファイル
##############################################


import socket
import time

class Fx3u:
    
    ##Fx3uクラス初期設定
    ##(self, 相手IPアドレス(String), 相手ポート番号(int), 送信サイズ(int))
    def __init__(self, ip, port, bufsize):
        self.ip = ip
        self.port = port
        self.bufsize = bufsize
        
        
    ##読出し(ワード単位)
    ##PLCからの返信を戻り値
    ##read(デバイス番号(str), デバイス点数(int))
    def read_worddevice(self, device, device_point):
        time.sleep(0.1)
        try:
            ##PLCとソケット通信
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.connect((self.ip, self.port))
            
            ##デバイス種類指定
            if device[0:1] == 'M':
                msg = '00FF000A4D20'
                
            elif device[0:1] == 'D':
                msg = '01FF000A4420'
            
            ##先頭デバイス指定
            msg = msg + format(int(device[1:4]), '08x')
            ##デバイス点数指定
            msg = msg + format(int(device_point), '02x') + '00'
            ##バイト型に変換
            msg = msg.encode('latin-1')
            #print(type(msg))

            ##PLCに送信
            client.send(msg)

            ##PLCからの返信情報
            response = client.recv(1024)

            response = response.decode()

            client.close()
            
            return response
        
        
            
        except ConnectionRefusedError:
            print("PLCに接続できませんでした")
    
    ##読出し(ビット単位)
    ##PLCからの返信を戻り値
    ##read(デバイス番号(str))
    def read_bitdevice(self, device):
        time.sleep(0.1)
        try:
            ##PLCとソケット通信
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.connect((self.ip, self.port))
            
            ##デバイス種類指定
            if device[0:1] == 'M':
                msg = '00FF000A4D20'
            
            ##先頭デバイス指定
            msg = msg + format(int(device[1:4]), '08x')
            ##デバイス点数指定
            msg = msg + '0400'
            ##バイト型に変換
            msg = msg.encode('latin-1')

            ##PLCに送信
            client.send(msg)

            ##PLCからの返信情報
            response = client.recv(1024)

            response = response.decode()

            client.close()
            
            return response
            
        except ConnectionRefusedError:
            print("PLCに接続できませんでした")
        
    ##書込み(ワード単位)
    ##PLCからの返信を戻り値
    ##read(デバイス番号(str), デバイス点数(int), 書込み内容(int))  
    def write_worddevice(self, device, device_point, write_content):
        time.sleep(0.1)
        try:
            ##PLCとソケット通信
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.connect((self.ip, self.port))
            
            ##デバイス種類指定
            if device[0:1] == 'M':
                msg = '02FF000A4D20'
                
            elif device[0:1] == 'D':
                msg = '03FF000A4420'
            
            ##先頭デバイス指定
            msg = msg + format(int(device[1:4]), '08x')
            ##デバイス点数指定
            msg = msg + format(int(device_point), '02x') + '00'
            #書き込み内容
            msg = msg + format(int(write_content), '04x')
            #バイト型に変換
            msg = msg.encode('latin-1')

            #PLCに送信
            client.send(msg)

            #PLCからの返信情報
            response = client.recv(1024)

            response = response.decode()

            client.close()
            ##PLCからの返信を戻り値に
            return response
            
        except ConnectionRefusedError:
            print("PLCに接続できませんでした")
            
            
    ##書込み(ビット単位)
    ##PLCからの返信を戻り値
    ##read(デバイス番号(str), デバイス点数(int), 書込み内容(int))  
    def write_bitdevice(self, device, write_content):
        time.sleep(0.1)
        try:
            ##PLCとソケット通信
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.connect((self.ip, self.port))
            
            ##デバイス種類指定
            if device[0:1] == 'M':
                msg = '02FF000A4D20'
            
            ##先頭デバイス指定
            msg = msg + format(int(device[1:4]), '08x')
            ##デバイス点数指定
            msg = msg + '0400'
            #書き込み内容
            msg = msg + str(write_content) +'000'
            #バイト型に変換
            msg = msg.encode('latin-1')

            #PLCに送信
            client.send(msg)

            #PLCからの返信情報
            response = client.recv(1024)

            response = response.decode()

            client.close()
            ##PLCからの返信を戻り値に
            return response
            
        except ConnectionRefusedError:
            print("PLCに接続できませんでした")
            