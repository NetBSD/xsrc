  XFree86 の構築
  David Dawes
  25 June 1999
  The X Japanese Documentation Project
  3 August 1999

  この文書では XFree86 をソース 配布物から構築する方法を説明します。この
  文書は全ソース配布物からの構築方法とともに、X サーバだけ構築するように
  切り詰めたソース配布物からの構築についても説明します。この文書は OS 固
  有の README ファイルと併せて読むように意図して書いてあります。
  ______________________________________________________________________

  目次

  1. ソース配布物からの XFree86 の構築
     1.1 XFree86 3.3.4 のソースの取得方法
     1.2 構築前のソースの設定
     1.3 配布物の構築とインストール

  2. サーバの再設定 (ソース配布物の場合)
  3. サーバの再設定 (バイナリ配布物の場合)
  4. 日本語訳について

  ______________________________________________________________________

  1.  ソース配布物からの XFree86 の構築

       注意:  XFree86 を構築しようとする前に適切な OS 固有の README
       ファイルを参照してください。OS 固有の README ファイルには、
       その OS の下で構築をうまく行うために必要な追加情報が書かれて
       います。

  XFree86 を構築の際には GCC-2 を使うことを強くお勧めします。GCC-2 は
  prep.ai.mit.edu 等の GNU ソースアーカイブサイトで入手できます。
  gcc-2.8.0 と egcs はどちらも、コードが何ヶ所かで壊れてしまうことが分
  かっています。特に、egcs は一部のモジュールのコンパイル時に最適化で問
  題を起こすようです。

  1.1.  XFree86 3.3.4 のソースの取得方法

  XFree86 のソースの入手のスタート地点はいくつかあります。そのうちの1つ
  は、XFree86 3.3.4 のソースアーカイブから直接作業を開始することです。こ
  の場合の手順は以下のようになります:

  o  XFree86 3.3.4 のソースは X334src-1.tgz, X334src-2.tgz,
     X334src-3.tgz に含まれています。これらのファイルは
     ftp://ftp.xfree86.org/pub/XFree86/3.3.4/source/や XFree86 のミラー
     サイトの同様のディレクトリにあります。 X334src-2.tgz はフォントと文
     書のソースです。 X334src-3.tgz は印刷可能な形式の文書です。
     X334src-1.tgz には他のものが全て入っています。文書やフォントが必要
     ない場合は、X334src-1.tgz だけ入手してもかまいません。

  o  空き容量が十分あるファイルシステムのディレクトリで次のコマンドを実
     行し、これらのファイルを展開してください。(ソース全てを展開するには
     約 140MB, コンパイルしてバイナリを得るには同程度の空きスペース必要
     です):

          gzip -d < X334src-1.tgz | tar vxf -
          gzip -d < X334src-2.tgz | tar vxf -
          gzip -d < X334src-3.tgz | tar vxf -

  別の方法は、X11R6.3 ソース配布物を入手し、これにパッチを当てて XFree86
  3.3.3 にし、さらにパッチを当てて XFree86 3.3.4 にすることです(後述)。
  これは次の手順で行います:

  o  公式パッチ 1, 2が当たった(ただし、3は当てない) X コンソーシアムの
     X11R6.3 配布を用意します。これは The Open Group の X のホームページ
     <http://www.opengroup.org/tech/desktop/x/>からのリンクをたどって入
     手することができます。

  o  ファイル R6.3pl2-3.3.3.diff1.gz, R6.3pl2-3.3.3.diff2.gz,
     R6.3pl2-3.3.3.diff3.gz, R6.3pl2-3.3.3.diff4.gz を
     ftp://ftp.xfree86.org/pub/XFree86/3.3.3/patches/ (またはミラーサイ
     トの同様のディレクトリ)から入手します。このソースを XFree86 3.3.3
     に更新するには、X11R6.3 pl2 のソースツリー内の xc ディレクトリがあ
     るディレクトリで以下のコマンドを実行します。

               gzip -d < R6.3pl2-3.3.3.diff1.gz | patch -p0 -E
               gzip -d < R6.3pl2-3.3.3.diff2.gz | patch -p0 -E
               gzip -d < R6.3pl2-3.3.3.diff3.gz | patch -p0 -E
               gzip -d < R6.3pl2-3.3.3.diff4.gz | patch -p0 -E

  この操作は必ず、全く変更されていないソースツリーに対して行ってくださ
  い。そうでなければ、当たらないパッチが出てくるかもしれません。

  さらに別の方法として、XFree86 3.3.3 のソースを用意し、これにパッチを当
  てて XFree86 3.3.4 にすることもできます。この場合には以下の手順を行い
  ます:

  o  この方法を使う場合には、既に XFree86 3.3.3 のソースをお持ちのことと
     思います。3.3.3 への公式パッチを1つでも当てていたら、 3.3.4 への
     アップグレードを始める前に、これらのパッチを外してください。

  o  3.3.3-3.3.4.diff.gz ファイルを
     ftp://ftp.xfree86.org/pub/XFree86/3.3.4/patches/ (またはミラーサイ
     トの同様のディレクトリ)から入手します。このソースをアップグレードし
     て XFree86 3.3.4 にするには、XFree86 3.3.3 のソースツリー内の xc
     ディレクトリがあるディレクトリから次のコマンドを実行します:

               gzip -d < 3.3.3-3.3.4.diff.gz | patch -p0 -E

  パッチ当ては必ず修正をしていないソースツリーで行ってください。さもなけ
  れば、うまく当たらないパッチがあるかもしれません。

  XFree86 の X サーバだけを構築したい場合は、``servers only'' 配布物と呼
  ばれる XFree86 ソースファイルを切り詰めたものを使用します。この方法を
  選択した場合は、次の手順を実行します:

  o  X334servonly.tgz というファイルを
     ftp://ftp.xfree86.org/pub/XFree86/3.3.4/source/ から (またはミラー
     サイトの同様な場所から)入手します。

  o  次の手順でファイルを展開します:

               gzip -d < X334servonly.tgz | tar vxf -

  XFree86 は X コンソーシアムの X11R6.1 contrib 配布物をほんの一部だけサ
  ポートしています。これを構築したい場合は、配布物のうち、少なくとも次の
  ファイルやディレクトリが必要です:

               contrib/Imakefile
               contrib/programs/Imakefile
               contrib/programs/ico
               contrib/programs/listres
               contrib/programs/showfont
               contrib/programs/viewres
               contrib/programs/xbiff
               contrib/programs/xcalc
               contrib/programs/xditview
               contrib/programs/xedit
               contrib/programs/xev
               contrib/programs/xeyes
               contrib/programs/xfontsel
               contrib/programs/xgc
               contrib/programs/xload
               contrib/programs/xman
               contrib/programs/xmessage

  XFree86 のパッチ contrib-3.3.3.diff.gz も必要になるでしょう。このパッ
  チを適用するには、contrib ディレクトリがあるディレクトリから以下のコマ
  ンドを実行してください:

               gzip -d < contrib-3.3.3.diff.gz | patch -p0 -E

  他の方法もあります。その場合、XFree86 ソースディレクトリから X333con-
  trib.tgz というファイルを入手して、次のコマンドで展開します:

               gzip -d < X333contrib.tgz | tar vxf -

  xtest 配布物を構築したい場合は、XFree86 ソースディレクトリから
  X33test.tgz ソース配布物 を入手して次のコマンドで展開します:

               gzip -d < X33test.tgz | tar vxf -

  注意ですが、xtest は (X11R6.3 から) X11 のコア配布物には含まれなくなり
  ました。

  1.2.  構築前のソースの設定

  設定作業の一番始めには、xc/config/cf ディレクトリに移動して、
  xf86site.def ファイルを host.def にコピーしましょう。そしてhost.def
  ファイルを全て読み(このファイルにはコメントがたくさん書かれています)、
  目的の設定に合わせてパラメタを設定します。普通は使っている OS に関連す
  る .cf を調べれば、デフォルトの設定が分かります。

  前のバージョンと異なる点があります。このバージョンでは imake が各種
  OS*Version を自動的に検出して設定するようになったので、これに関する設
  定を明示的に入力する必要はなくなりました。

  ソース配布物のうち X334src-1.tgz の部分だけを使う場合には、 BuildFonts
  は NO と定義する必要があります。

  ``servers only'' 配布物を使用する場合は、BuildServersOnly を YES と定
  義する必要があります。

  1.3.  配布物の構築とインストール

  配布物を構築する前には、お使いの OS 固有の README ファイルを通読してく
  ださい。このファイルは xc/programs/Xserver/hw/xfree86/doc にありま
  す。OS 固有の詳細な注意点が理解できれば、xc ディレクトリに移動して
  ``make World'' を実行します。 実行の際には必要に応じて、OS 固有の
  README ファイルに書かれている BOOTSTRAPCFLAGS を使用してください。標準
  出力と標準エラー出力を World.Log にリダイレクトしておくと、構築中に発
  生した問題の原因追求に役立ちます。

  構築が終了したら、 World.Log を調べて問題が無いことを確認してくださ
  い。問題がなければバイナリをインストールしても良いでしょう。全てのソー
  ス配布物を使う場合は、xc ディレクトリからインストールを行います。
  ``servers only'' 配布物を使用している場合は、 xc/programs/Xserver ディ
  レクトリからインストールを行います。インストールを行うには``make
  install'' と ``make install.man'' を実行します。インストールに十分なだ
  けの空き容量が /usr/X11R6 にあることを確認してください。/usr 以外の
  ファイルシステムにインストールする場合にはインストールの前に
  /usr/X11R6 へシンボリックリンクを張ってください。

  /usr/X11R6/lib/Server にあるバイナリの LinkKit をインストールするに
  は、``make install.linkkit'' を xc ディレクトリから実行してください。

  XFree86 のサポートしている contrib リリースの一部を構築するには、まず
  コア配布物がインストールしてあることを確認してください。次に、 contrib
  ディレクトリに移動して ``xmkmf -a; make'' を実行します。コンパイルが終
  了したら ``make install'' と ``make install.man'' を実行してインストー
  ルを行います。

  xtest 配布物を構築/実行するには、test/xsuite/NOTES.xf86 ファイルにある
  手順を参照してください。

  2.  サーバの再設定 (ソース配布物の場合)

  複数の異なるサーバや異なるドライバを含むサーバを構築するには、以下の手
  順を行います:

  1. 新しいドライバのソースが正しい位置にあることを確認します(例えば
     SVGA サーバのドライバのソースは
     xc/programs/Xserver/hw/xfree86/vga256/drivers というサブディレクト
     リにあるはずです)。

  2. host.def ファイル中でサーバの定義の設定を変更し、構築するサーバを指
     定します。また、必要に応じてドライバのリストを変更します。

  3. xc/programs/Xserver ディレクトリで以下のコマンドを実行します:

               make Makefile
               make Makefiles
               make depend
               make

  3.  サーバの再設定 (バイナリ配布物の場合)

  サーバのバイナリリンクキットを導入していれば、ドライバとサーバ内の一部
  の機能拡張を再設定することができます。この作業の詳細な手順について
  は、README.LinkKit ファイルを参照してください。

  $XFree86: xc/programs/Xserver/hw/xfree86/doc/Japanese/BUILD,v 1.1.2.3 1999/11/26 15:22:58 hohndel Exp $

  4.  日本語訳について

  日本語訳は X Japanese Documentation Project が行いました( 翻訳(XFree86
  3.3): 岡本一幸 <ikko-@pacific.rim.or.jp>
  <mailto:ikko-@pacific.rim.or.jp>, 更新(XFree86 3.3.3, 3.3.4 対応): 藤
  原輝嘉 <fujiwara@linux.or.jp> <mailto:fujiwara@linux.or.jp>, 校正: 金
  田浩司  <vanitas@ma3.seikyou.ne.jp>
  <mailto:vanitas@ma3.seikyou.ne.jp>)。問題点の指摘やご意見は藤原まで御
  連絡ください。原文の著作権は XFree86 プロジェクト社にあり、日本語訳の
  著作権は X Japanese Documentation Project にあります。

