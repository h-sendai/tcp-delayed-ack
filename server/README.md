# serverプログラム
```
Usage: server [-b bufsize (1460)] [-s sleep_usec (0)] [-q] [-r rate] [-S so_sndbuf]
-b bufsize:    one send size (may add k for kilo, m for mega)
-s sleep_usec: sleep useconds after write
-p port:       port number (1234)
-q:            enable quick ack
-r rate:       data send rate (bytes/sec).  k for kilo, m for mega
-S so_sndbuf:  set socket send buffer size
-R sleep_sec:  sleep_sec after receive SIGUSR1
```

# 起動例

```
./server -b 16k -r 30m
```

一度に16kBソケットにwrite()することでデータを送る。それぞれのwrite()の
時間間隔を調節することで全体として30MB/sのレートでデータを送る。

クライアントプログラムの接続後、子プロセスが生成されそれがデータを
送る。 SIGUSR1を受け取とるとデータ送信を一時的に停止する（デフォルトは
5秒間。-Rオプションでデータ送信をしない時間を設定できる）。

