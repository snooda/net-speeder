# net-speeder
net-speeder 在高延迟不稳定链路上优化单线程下载速度 

1、过滤icmp重复发包

2、修复windows系统pptp拔不上的问题

3、加入只重复发送增倍模式

项目由https://code.google.com/p/net-speeder/ 迁入

A program to speed up single thread download upon long delay and unstable network

在高延迟不稳定链路上优化单线程下载速度

注：net-speeder不依赖ttl的大小，ttl的大小跟流量无比例关系。不存在windows的ttl大，发包就多的情况。

运行时依赖的库：libnet，libpcap

debian/ubuntu安装libnet：

    安装libpcap： apt-get install libnet1 libpcap0.8

编译需要安装libnet和libpcap对应的dev包

    安装libpcap-dev： apt-get install libnet1-dev libpcap0.8-dev 

centos安装： 下载epel：https://fedoraproject.org/wiki/EPEL/zh-cn 例：CentOS6 64位：

wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm

如果是centos5，则在epel/5/下

然后安装epel：rpm -ivh epel-release-X-Y.noarch.rpm

然后即可使用yum安装：yum install libnet libpcap libnet-devel libpcap-devel

安裝：make install

卸載：make uninstall

使用方法(需要root权限启动）：

参数：./net_speeder 模式 网卡名 加速规则（bpf规则）[本地网卡IP]

模式可以为: auto(默认), normal, cooked

在自动模式下，如果网卡名为venet*，会自动选择cooked模式。cooked模式适用于OpenVZ，normal适合其他电脑

最简单用法（适用于OpenVZ）： # ./net_speeder auto venet0 ip 加速所有ip协议数据
