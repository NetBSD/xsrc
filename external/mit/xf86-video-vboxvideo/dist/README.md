xf86-video-vboxvideo - VirtualBox video driver for the Xorg X server
--------------------------------------------------------------------

This driver is only for use in VirtualBox guests without the
vboxvideo kernel modesetting driver in the guest kernel, and
which are configured to use the VBoxVGA device instead of a
VMWare-compatible video device emulation.

Guests with the vboxvideo kernel modesetting driver should use the
Xorg "modesetting" driver module instead of this one.

All questions regarding this software should be directed at the
Xorg mailing list:

  https://lists.x.org/mailman/listinfo/xorg

The primary development code repository can be found at:

  https://gitlab.freedesktop.org/xorg/driver/xf86-video-vbox

Please submit bug reports and requests to merge patches there.

For patch submission instructions, see:

  https://www.x.org/wiki/Development/Documentation/SubmittingPatches

This driver is dedicated to the memory of Michael Thayer, who brought
it to X.Org and made the initial release before he passed away in 2019.
