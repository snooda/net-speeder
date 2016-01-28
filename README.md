# net-speeder
net-speeder 在高延迟不稳定链路上优化单线程下载速度 

项目由https://code.google.com/p/net-speeder/  迁入


A program to speed up single thread download upon long delay and unstable network

在高延迟不稳定链路上优化单线程下载速度

注1：开启了net-speeder的服务器上对外ping时看到的是4倍，实际网络上是2倍流量。另外两倍是内部dup出来的，不占用带宽。
另外，内部dup包并非是偷懒未判断。。。是为了更快触发快速重传的。
注2：net-speeder不依赖ttl的大小，ttl的大小跟流量无比例关系。不存在windows的ttl大，发包就多的情况。


安装步骤：

1：下载源码并解压

    wget https://github.com/snooda/net-speeder/archive/master.zip
    unzip master.zip

2：准备编译环境

debian/ubuntu：

    #安装libnet-dev：
    apt-get install libnet1-dev
    #安装libpcap-dev：
    apt-get install libpcap0.8-dev 

centos： 

    #下载epel：https://fedoraproject.org/wiki/EPEL/zh-cn 例：CentOS6 64位：
    wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm
    #（如果是centos5，则在epel/5/下）
    #安装epel：
    rpm -ivh epel-release-6-8.noarch.rpm
    #然后即可使用yum安装：
    yum install libnet libpcap libnet-devel libpcap-devel

编译：

Linux Cooked interface使用编译（venetX，OpenVZ）：

    sh build.sh -DCOOKED

普通网卡使用编译（Xen，KVM，物理机）：

    sh build.sh

使用方法(需要root权限启动）：

    #参数：./net_speeder 网卡名 加速规则（bpf规则）
    #ovz用法(加速所有ip协议数据)： 
    ./net_speeder venet0 "ip"
