#! /bin/sh
for i in sgml/*.sgml 
 do
   sgml2txt -f -l ja -c nippon $i
 done

# OS Docs
mv Bsdi.txt README.Bsdi
mv FreeBSD.txt READ.FreeBSD
mv NetBSD.txt README.NetBSD
mv OpenBSD.txt README.OpenBSD
mv Mach.txt README.Mach
mv Linux.txt README.Linux
mv LynxOS.txt README.LynxOS
# mv Minix.txt README.Minix
# mv Amoeba.txt README.Amoeba
mv SCO.txt README.SCO
mv isc.txt README.isc
mv OS2.txt README.OS2
mv OS2note.txt OS2.Notes
mv SVR3.txt README.SVR3

mv SOLX86.txt README.SOLX86
mv SVR4.txt README.SVR4

# /* Hardware docs */
mv DECtga.txt README.DECtga
mv I128.txt README.I128
mv Mach32.txt READ.Mach32
mv Mach64.txt READ.Mach64
mv MGA.txt README.MGA
mv NVIDIA.txt README.NVIDIA
mv Oak.txt README.Oak
mv P9000.txt README.P9000
mv Video7.txt README.Video7
mv S3.txt README.S3
mv S3V.txt README.S3V
mv SiS.txt README.SiS
mv W32.txt README.W32
mv WstDig.txt README.WstDig
mv apm.txt README.apm
mv ark.txt README.ark
mv agx.txt README.agx
mv ati.txt README.ati
mv chips.txt README.chips
mv cirrus.txt README.cirrus
mv cyrix.txt README.cyrix
mv epson.txt README.epson
mv mouse.txt README.mouse
mv neo.txt README.neo
mv rendition.txt README.rendition
mv 3DLabs.txt README.3DLabs
mv trident.txt README.trident
mv tseng.txt README.tseng
mv fbdev.txt README.fbdev

/* Main docs */
mv CPYRIGHT.txt COPYRIGHT
mv RELNOTE.txt RELNOTES
mv Config.txt README.Config
mv BUILD.txt BUILD
mv README.txt README
mv QStart.txt QuickStart.doc
rm DocIndex.txt
mv INSTALL.txt INSTALL

/* Other docs */
mv LinkKit.txt LinkKit
mv clkprog.txt README.clkprog
mv xinput.txt xinput
mv VidModes.txt VideoModes.doc
mv VGADriv.txt VGADriver.doc

