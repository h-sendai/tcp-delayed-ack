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

# バージョン調査

debianではsnapshot.debian.orgにそれまで作られたパッケージが
のこっていて/etc/apt/sources.listで日時を指定してその時点を
模擬してapt installするなどができる。
debian 12では、以下のようにして2023年12月31日23:59:59の時点での
パッケージをインストールすることができる。

```
deb     https://snapshot.debian.org/archive/debian/20231231T235959Z/ bookworm main
deb-src https://snapshot.debian.org/archive/debian/20231231T235959Z/ bookworm main
deb     [check-valid-until=no] https://snapshot.debian.org/archive/debian-security/20231231T235959Z/ bookworm-security main
deb-src [check-valid-until=no] https://snapshot.debian.org/archive/debian-security/20231231T235959Z/ bookworm-security main
```

上のようにsources.listを変更後、apt updateして、
apt-cache search linux-imageで取得可能なカーネルパッケージ一覧がとれる。

いくつかやってみると

- 6.1.0-13までは40ミリのディレイが発生する
- 6.1.0-15では40ミリのディレイは発生しない

ということがわかった
(KVM guestでdebian 12を稼働、
サーバーはKVMホストのAlmaLinux 9 5.14.0-570.49.1.el9_6.x86_64で
テストした。kvm guestでもテスト可能だった)。

6.1.0-14というのはapt-cache search linux-imageでは出てこなかったので
セットできなかった。

https://www.debian.org/News/2023/20231210 には

> Please be advised that this document has been updated as best to
> reflect Debian 12.3 being superseded by Debian 12.4. These changes
> came about from a last minute bug advisory of #1057843 concerning
> issues with linux-image-6.1.0-14 (6.1.64-1).

とある。#10578436 というのは
https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1057843
でext4 data corruption in 6.1.64-1らしい。

/bootにあるconfigをみると

- 6.1.0-13はLinux/x86 6.1.55
- 6.1.0-15はLinux/x86 6.1.66

となっている。

Debianでは以下のようにするとカーネルを自分でコンパイルして
セットすることができる。

- build-essentialをいれたあと、
```
bc bison flex libelf-dev libssl-dev libncurses-dev dwarves rsync
```
をいれる。
- make olddefconfig (/boot/にあるconfigからconfigを作る）
- make localmodconfig (ロードされているモジュールのみを有効可する)
- make -j$(nproc) bindeb-pkg

これで一つ上のディレクトリに
```
linux-headers-6.1.55_6.1.55-7_arm64.deb
linux-image-6.1.55-dbg_6.1.55-7_arm64.deb
linux-image-6.1.55_6.1.55-7_arm64.deb
linux-libc-dev_6.1.55-7_arm64.deb
linux-upstream_6.1.55-7_arm64.buildinfo
linux-upstream_6.1.55-7_arm64.changes
```
ができる。dpkg -i linux-image-6.1.55_6.1.55-7_arm64.deb でインストールする。
update-grubは自動で走るのでリブート時にgrubメニューから起動するカーネルを
選択する。

6.1.55と6.1.66の間をbisectすると

- 6.1.56ではディレイが入る
- 6.1.57ではディレイは入らない

という結果になった。

それぞれのキャプチャを
- client-linux6.1.56.cap
- client-linux6.1.57.cap
としてこのディレクトリにいれてみた。kvmではなくて実PCで
キャプチャした。

6.1.57のChangeLogをみると

4acf07bafb5812162ab41dc0ef4e8d9f798b5fc0

のコミットか?
6.1.xのほかにもこのコミットがはいっているかもしれない。



