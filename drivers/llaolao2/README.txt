使用驱动切换GDK8的JTAG信号

编译驱动：
make

加载驱动：
sudo insmod llaolao.ko

切换JTAG：
echo jtag | sudo tee -a /proc/llaolao