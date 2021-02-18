##开机后在登录界面，鼠标键盘失灵

上次折腾完神船触摸板的驱动之后，开机之后除了触摸板其他都用不了。

重装xserver-xorg-input-all后解决:

```html
因为安装别的软件时,误删了xserver-xorg-input-all，所以重新安装下即可
# sudo apt install xserver-xorg-input-all
如果还不行,就重新安装下桌面，有可能也被误删.
# sudo apt install ubuntu-desktop
```