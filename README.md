# TCP Delayed Ack

## TCP_QUICKACK

TCPにはできるだけackのみのパケットを送らないようにする
delayed ackの機能がある。一定時間待って、送るデータが
なければackのみ送る。
「一定時間」は昔の実装だと200ミリ秒だった。
現在のLinuxではHZで違うようなかんじだがAlmaLinux 8で
試してみると40ミリ秒だった。

ただし、デフォルトではTCP_QUICKACKが有効になっていて
一定時間待つことなしにすぐにackを送るような動作を
しているような気がする（ので調べるためにここにある
プログラムをかいてtcpdumpで見てみることにした）。

## TCP_QUICKACKの値の取得

[TCP_QUICKACKのデフォルト値を取得するプログラム](quickack-default-value/)
を書いてみた。ソケットを作ってすぐに``getsockopt()``で
``TCP_QUICKACK``の値を取得している。
AlmaLinux 8で試すと``1``(有効化されている)が表示された。

## Delayed Ackの様子をみるプログラム

tcpdumpでdelayed ackの様子を観察するプログラム。

- サーバーは起動して、read()で待つ。
- クライアントは起動して接続するとwrite()で1バイト書き、
  続けてread()で待つ。
- サーバーは1バイトread()して1バイトwrite()する。
- クライアントはread()で1バイト読む。
- クライアントは1秒sleep()する。
- 以下繰り返し。何回繰り返すかは-nオプションで指定する。

```
クライアント            サーバー

1バイト ----------->
        <----------- 1バイト

(スリープ1秒)

1バイト ----------->
        <----------- 1バイト
```

## 起動

サーバー
```
./server
```

クライアント

```
./client [-q 0 or 1] remote-host
```

クライアント側には``-q``オプションでTCP_QUICKACK
の値を指定するオプションをつけた。
``./client -q 0``と指定するとTCP_QUICKACKが無効となる。
指定しない場合はTCP_QUICKACKはシステムデフォルトの値に
なっている。AlmaLinux 8では1であった。

```
Usage: client [options] ip_address
Connect to server and read data.  Display through put before exit.

options:
-c CPU_NUM      running cpu num (default: none)
-p PORT         port number (default: 1234)
-q VALUE        specify quickack value (default: -1: use default value)
-n n_data       number of data (each 20 bytes) from server.  (default: 2)
-h              display this help
```

(注)
```
quickack(0)

for ( ; ; ) {
    write()
    read()
}
```
とすると最初のループのwrite(), read()はquickackが0でまわるが
次からはデフォルト値のquickack = 1でまわる。
必要なら毎回quickackの値をセットする。(注おわり)

## AlmaLinux 8 client <----> RHEL 9 Server

サーバー:
```
./server
```

クライアント:
```
./client -q 0 -n 50 -s 500000 remote-host
```
として起動したときのtcpdumpのログを
[client-on-AlmaLinux8.txt](client-on-AlmaLinux8.txt)
においた。

192.168.10.200がサーバー、192.168.10.201がクライアント:
```
0.500787 0.458605 IP 192.168.10.201.51786 > 192.168.10.200.1234: Flags [P.], seq 2:3, ack 2, win 229, length 1
0.500927 0.000140 IP 192.168.10.200.1234 > 192.168.10.201.51786: Flags [P.], seq 2:3, ack 3, win 502, length 1
0.542176 0.041249 IP 192.168.10.201.51786 > 192.168.10.200.1234: Flags [.], ack 3, win 229, length 0
```
0.500927秒に受け取ったデータのackを41.249秒後に送っていることがわかる。

クライアント側で``-q``オプションをつけないで起動したときのtcpdumpのログを
[client-on-AlmaLinux8-quickack.txt](client-on-AlmaLinux8-quickack.txt)
においた。

192.168.10.200がサーバー、192.168.10.201がクライアント:
```
0.500834 0.500206 IP 192.168.10.201.49816 > 192.168.10.200.1234: Flags [P.], seq 2:3, ack 2, win 229, length 1
0.500972 0.000138 IP 192.168.10.200.1234 > 192.168.10.201.49816: Flags [P.], seq 2:3, ack 3, win 502, length 1
0.500994 0.000022 IP 192.168.10.201.49816 > 192.168.10.200.1234: Flags [.], ack 3, win 229, length 0
```
0.00972秒に受け取ったデータのackを0.000022秒後に送っていることがわかる。

## delayed ackの統計

``netstat -s | grep dela``でdelayed ackがおきた回数が取得できる。
(netstatはAlmaLinuxではnet-toolsパッケージに入っている)。

(例)
```
% netstat -s | grep dela
    5130 delayed acks sent
    5 delayed acks further delayed because of locked socket
% cd client/
% ./client -q 0 -n 10 -s 500000 remote-host
10:46:56.145767 client: SO_RCVBUF: 87380 (init)
10:46:56.146463 client: quickack: 0
10:46:56.146489 client: write start
(略)
10:47:00.649898 client: entering sleep
10:47:01.149987 client: sleep done
% netstat -s | grep dela
    5140 delayed acks sent
    5 delayed acks further delayed because of locked socket
```

server, clientを同一マシンで動かし、``lo``を通じて通信させるときにも
``client -q 0 127.0.0.1``とするとdelayed ackしていることを確認できる。

## FreeBSD client <----> Linux Server

FreeBSD 13.2でクライアントを

```
./client -n 50 -s 500000 linux-server
```

で起動したときのクライアント側(FreeBSD側)で取得した
[キャプチャデータ](client-on-FreeBSD.txt)。
length 0のところがdelayed ackになっていてデータを受信してから
40ミリ秒後にackをだしているのがわかる。

192.168.10.204がFreeBSD、192.168.10.201がAlmaLinux 8。

```
1.050133 0.475808 IP 192.168.10.204.51934 > 192.168.10.201.1234:  Flags [P.], seq 3:4, ack 3, win 1027, length 1
1.050326 0.000193 IP 192.168.10.201.1234  > 192.168.10.204.51934: Flags [P.], seq 3:4, ack 4, win 229, length 1
1.092328 0.042002 IP 192.168.10.204.51934 > 192.168.10.201.1234:  Flags [.], ack 4, win 1027, length 0
```

