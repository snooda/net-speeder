# net-speeder
net-speeder 在高延迟不稳定链路上优化单线程下载速度 

过滤icmp重复发包
修复windows系统pptp拔不上的问题

项目由https://code.google.com/p/net-speeder/  迁入


A program to speed up single thread download upon long delay and unstable network

在高延迟不稳定链路上优化单线程下载速度

注1：开启了net-speeder的服务器上对外ping时看到的是4倍，实际网络上是2倍流量。另外两倍是内部dup出来的，不占用带宽。
另外，内部dup包并非是偷懒未判断。。。是为了更快触发快速重传的。
注2：net-speeder不依赖ttl的大小，ttl的大小跟流量无比例关系。不存在windows的ttl大，发包就多的情况。

运行时依赖的库：libnet， libpcap

debian/ubuntu安装libnet：apt-get install libnet1

    安装libpcap： apt-get install libpcap0.8 

编译需要安装libnet和libpcap对应的dev包 debian/ubuntu安装libnet-dev：apt-get install libnet1-dev

    安装libpcap-dev： apt-get install libpcap0.8-dev 

centos安装： 下载epel：https://fedoraproject.org/wiki/EPEL/zh-cn 例：CentOS6 64位：

wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm

如果是centos5，则在epel/5/下

然后安装epel：rpm -ivh epel-release-X-Y.noarch.rpm

然后即可使用yum安装：yum install libnet libpcap libnet-devel libpcap-devel

编译：

make

安裝：
make install

卸載：
make uninstall

使用方法(需要root权限启动）：

参数：./net_speeder [模式] 网卡名 加速规则（bpf规则）

模式可以为: auto(默认), normal, cooked

在自动模式下，如果网卡名为venet*，会自动选择cooked模式。
cooked模式适用于OpenVZ，normal适合其他电脑

最简单用法（适用于OpenVZ）： # ./net_speeder venet0 "ip" 加速所有ip协议数据
