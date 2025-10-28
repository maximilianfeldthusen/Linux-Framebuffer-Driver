# Linux-Framebuffer-Driver




How to compile, install, and test your Linux framebuffer driver:

-----
# <a name="_gx4f1aab1kor"></a>**🛠️ How to Compile and Install a Linux Framebuffer Driver**
This guide walks you through compiling and installing the virtual framebuffer driver written in C.

-----
## <a name="_q9xxdd4c31s"></a>**📁 Step 1: Set Up Your Environment**
Make sure your system has the required tools and headers:

sudo apt update

sudo apt install build-essential linux-headers-$(uname -r)

-----
## <a name="_6en8oi47ok70"></a>**📦 Step 2: Create Your Driver Source File**
Create a working directory and save your driver code:

mkdir ~/vfb\_driver

cd ~/vfb\_driver

Save your code in a file named vfb.c.

-----
## <a name="_jquq847ad8y3"></a>**🧰 Step 3: Create a Makefile**
Create a file named Makefile with the following content:

obj-m += vfb.o

all:

`    `make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:

`    `make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

-----
## <a name="_4tygmhoo9knp"></a>**🧪 Step 4: Compile the Driver**
Run the following command to build the kernel module:

make

This will generate a file named vfb.ko.

-----
## <a name="_7iqm7j5vjgnj"></a>**🚀 Step 5: Install the Driver**
Use insmod to insert the module into the kernel:

sudo insmod vfb.ko

Check kernel messages to confirm:

dmesg | tail

You should see:

Virtual framebuffer device registered

-----
## <a name="_k0phr9btjjwj"></a>**🔍 Step 6: Verify the Device**
Check if the framebuffer device is available:

ls /dev/fb\*

You may see /dev/fb0 or /dev/fb1.

-----
## <a name="_3pfa5jfe4l95"></a>**🧹 Step 7: Uninstall the Driver**
To remove the module:

sudo rmmod vfb

Verify with:

dmesg | tail

You should see:

Virtual framebuffer device unregistered

-----
## <a name="_xrf476ibi43y"></a>**🧪 Step 8: Optional Testing**
You can write to the framebuffer using tools like dd or custom C programs.

Example: fill the screen with white pixels (assuming 32-bit color):

echo -ne '\xff\xff\xff\xff' | dd of=/dev/fb0 bs=4 count=$((800\*600))

-----
## <a name="_u1fdlnnngvrn"></a>**✅ Summary**

|**Step**|**Description**|
| :-: | :-: |
|1|Install build tools and headers|
|2|Save driver code as vfb.c|
|3|Create a Makefile|
|4|Compile with make|
|5|Load with insmod|
|6|Verify with ls /dev/fb\*|
|7|Unload with rmmod|
|8|Optionally test with dd|

-----

