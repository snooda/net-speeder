# net-speeder
net-speeder 在高延迟不稳定链路上优化单线程下载速度 

项目由https://code.google.com/p/net-speeder/  迁入


关注微信公众号了解最新开发进度/获取帮助/提出建议：

<img src="http://www.snooda.com/images/qrcode.jpg" />


A program to speed up single thread download upon long delay and unstable network

在高延迟不稳定链路上优化单线程下载速度

注1：开启了net-speeder的服务器上对外ping时看到的是4倍，实际网络上是2倍流量。另外两倍是内部dup出来的，不占用带宽。
另外，内部dup包并非是偷懒未判断。。。是为了更快触发快速重传的。
注2：net-speeder不依赖ttl的大小，ttl的大小跟流量无比例关系。不存在windows的ttl大，发包就多的情况。

使用方法(需要root权限启动）：

    #参数：./net_speeder 网卡名 加速规则（bpf规则）
    #ovz用法(加速所有ip协议数据)： 
    ./net_speeder venet0 "ip"
    #普通网卡用法
    ./net_speeder eth0 "ip"

安装步骤：

1：准备编译环境

debian/ubuntu：

    sudo apt-get install libnet1-dev libpcap0.8-dev git gcc
    
centos： 

    # yum
    sudo yum install libnet libpcap libnet-devel libpcap-devel git gcc
    # dnf
    sudo dnf install libnet libpcap libnet-devel libpcap-devel git gcc
    
2：克隆源码

    # 定位至你的安装目录
    cd ~
    git clone https://github.com/snooda/net-speeder.git

3：编译
    
    cd net-speeder 
    
Linux Cooked interface使用编译（venetX，OpenVZ）：

    sh build.sh -DCOOKED

普通网卡使用编译（Xen，KVM，物理机）：

    sh build.sh
    
4：配置 systemd

键入 `vi /usr/lib/systemd/system/net-speeder.service` 指令进行编辑
  
    [Unit]
    Description=net-speeder
    After=network.target
    Wants=network.target

    [Service]
    Type=simple
    PIDFile=/var/run/net_speeder.pid
    # 有需要请编辑这里
    ExecStart=/root/net-speeder/net_speeder venet0 "ip"

    [Install]
    WantedBy=multi-user.target
    
然后执行以下指令：

    systemctl enable net-speeder
    systemctl start net-speeder
    
    
更新方法（可写入 .sh 并设定 crontab 以自动完成)：

    cd ~/net-speeder
    git pull
    # 或 sh build.sh
    sh build.sh -DCOOKED 
