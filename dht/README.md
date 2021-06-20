```bash
sudo apt-get update
```



#如果之前没有安装过 git，执行下面的命令安装一下
```bash
sudo apt-get install git-core
```



# 下载 wiringPi

git clone https://github.com/WiringPi/WiringPi
#编译 wiringPi
cd WiringPi
sudo ./build

# 编译程序并运行

```bash
gcc -Wall -o dht11 dht11.c -lwiringPi
sudo ./dht11
```



# 实验原理

