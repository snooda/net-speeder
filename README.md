# net-speeder
net-speeder 在高延迟不稳定链路上优化单线程下载速度 

项目由https://code.google.com/p/net-speeder/  迁入
A program to speed up single thread download upon long delay and unstable network


###debian/ubuntu：

    apt-get install libnet1
    apt-get install libpcap0.8 
    apt-get install libnet1-dev
    apt-get install libpcap0.8-dev
    apt-get install unzip
    wget https://github.com/snooda/net-speeder/archive/master.zip
    unzip master.zip
    cd net-speeder-master
    sh build.sh -DCOOKED
    ./net_speeder 网卡名 "ip"

###centos6：

    wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm
    rpm -ivh epel-release-6-8.noarch.rpm
    yum install libnet libpcap libnet-devel libpcap-devel
    sh build.sh -DCOOKED
    ./net_speeder 网卡名 "ip"
    
###centos5:

    wget http://dl.fedoraproject.org/pub/epel/5/x86_64/epel-release-6-8.noarch.rpm
    rpm -ivh epel-release-6-8.noarch.rpm
    yum install libnet libpcap libnet-devel libpcap-devel
    sh build.sh -DCOOKED
    ./net_speeder 网卡名 "ip"

###How to use:
参数：./net_speeder 网卡名 加速规则（bpf规则）
Example:

    ./net_speeder venet0 "ip"

加速所有ip协议数据 

###To run in the background using [screen][2]:
Command: “Ctrl-a”  “c” #create a new window

    ./net_speeder 网卡名 "ip"
Command: “Ctrl-a” “d” #detach From Screen

注1：开启了net-speeder的服务器上对外ping时看到的是4倍，实际网络上是2倍流量。另外两倍是内部dup出来的，不占用带宽。另外，内部dup包并非是偷懒未判断，是为了更快触发快速重传的。
注2：net-speeder不依赖ttl的大小，ttl的大小跟流量无比例关系。不存在windows的ttl大，发包就多的情况。
注3：用ifconfig查看网卡名

  [2]: http://www.rackaid.com/blog/linux-screen-tutorial-and-how-to/
