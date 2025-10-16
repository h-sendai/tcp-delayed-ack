# tcp-server-clientでのテスト

# 01.cap

client側環境は下の03.capと同一だが、カーネルが6.1.0-9-amd64というのが
残っていたのでgrubでこのカーネルを選択して起動した。

- server: AlmaLinux 9 (kernel 5.14.0-570.49.1.el9_6.x86_64)
- server-command: ./server -b 1k -D -s 100000 
- client: debian 12 (kernel 6.1.0-9-amd64)
- client-command: ./client -b 1k hspc02-exp0

クライアント側17個め以降はackが40ミリ秒にでる。

client側プログラムをtcp-server-client/client.cから改造して、read()の前後で
quickackが有効になっているかどうか
プリントさせてみた
[client.log.01.txt](client.log.01.txt)
。いつも有効になっている。


# 02.cap

serverは01.capをとったときと同一PC,同一プログラム、同一OS
clientはハードウェアは同一。OSもdebian 12だがカーネルが更新されている

- server: AlmaLinux 9 (kernel 5.14.0-570.49.1.el9_6.x86_64)
- server-command: ./server -b 1k -D -s 100000 
- client: debian 12 (kernel 6.1.0-26-amd64)
- client-command: ./client -b 1k hspc02-exp0

client側プログラムをtcp-server-client/client.cから改造して、read()の前後で
quickackが有効になっているかどうか
プリントさせてみた
[client.log.02.txt](client.log.02.txt)
。いつも有効になっている。

17個め以降、ackがでるのが40ミリ秒という事象はなかった。
kernel 6.1.0-9-amd64 -> 6.1.0-26-amd64でなにか変更があったのか?

# 03.cap

serverは01.capをとったときと同一PC,同一プログラム、同一OS
clientはハードウェアは同一。OSもdebian 12だがカーネルが更新されている

- server: AlmaLinux 9 (kernel 5.14.0-570.49.1.el9_6.x86_64)
- server-command: ./server -b 1k -D -s 100000 
- client: debian 12 (kernel 6.1.0-37-amd64)
- client-command: ./client -b 1k hspc02-exp0
