  XFree86 の構築
  David Dawes 著
  2 June 1997
  岡本 一幸 Kazuyuki Okamoto <ikko-@pacific.rim.or.jp> 訳
  29 July 1997

  この文書は如何に XFree86 を ソース 配布物から構築するか説明します。こ
  の文書は全ソース配布物と同様に X サーバだけ構築するように切り詰めたソ
  ース配布物から構築する説明も言及します。この文書は OS 固有の README
  ファイルと併せて使用するように設計しています。
  ______________________________________________________________________

  Table of Contents:

  1.      ソース配布物から XFree86 を構築する。

  1.1.    XFree86 3.3 のソースをどうやって入手するか。

  1.2.    構築前のソースの設定

  1.3.    配布物の構築と導入

  2.      サーバの再設定 (ソース配布物の場合)

  3.      サーバの再設定 (バイナリ配布物の場合)
  ______________________________________________________________________

  1.  ソース配布物から XFree86 を構築する。

        注意:  XFree86 を構築しようとする前に適切な OS 固有の README
        ファイルを参照してください。OS 固有の README ファイルはお手元の
        OS の下で構築がうまくいくのに必要な追加情報が含まれています。

  我々は XFree86 を構築するのに GCC-2 を使用することを強くお勧めします。
  2.6.0 または 2.4.5 以前の 2.4.x 版は構築や実行を阻害するバグがあるので
  使用しないでください。(gcc-2 は prep.ai.mit.edu かその他の GNU ソース
  アーカイブサイトにあります。)

  1.1.  XFree86 3.3 のソースをどうやって入手するか。

  XFree86 のソースを入手するのに幾つかの開始方法があります。一つ目は
  XFree86 3.3 ソースアーカイブから直接はじめることです。この場合は次の手
  順で行います。

  o  XFree86 3.3 のソースは X33src-1.tgz, X33src-2.tgz と X33src-3.tgz
     に含まれています。これらのファイルは
     ftp://ftp.xfree86.org/pub/XFree86/3.3/source/ と XFree86 のミラーサ
     イトの同様な場所にあります。X33src-2.tgz はフォントと文書のソースで
     す。X33src-3.tgz は印刷可能な形態の文書です。文書やフォントが必要な
     い場合は、 X33src-1.tgz だけ入手してください。

  o  十分容量に余裕があるファイルシステムのディレクトリで次の手順を実行
     してこれらのファイルを伸長してください。(全てのソースには約 130MB
     ,コンパイルしてバイナリを得るには同量の空きスペースが必要です。)

                gzip -d < X33src-1.tgz | tar vxf -
                gzip -d < X33src-2.tgz | tar vxf -
                gzip -d < X33src-3.tgz | tar vxf -

  他の X11R6.3 ソース配布物と XFree86 3.3 にパッチを当てて始める方法は次
  の手順で行います。

  o  公式パッチ 1 が当たった X コンソーシアムの X11R6.3 配布から始めま
     す。次の URL X のホームページ <http://www.x.org> からのリンクから得
     られます。

  o  R6.3pl1-3.3.diff1.gz, R6.3pl1-3.3.diff2.gz, R6.3pl1-3.3.diff3.gz,
     R6.3pl1-3.3.diff4.gz と cfont33.tgz を
     ftp://ftp.xfree86.org/pub/XFree86/3.3/patches/ から (または ミラー
     サイトの同様な場所から)入手してください。 XFree86 3.3 へソースを更
     新するには次の X11R6.3 pl1 のソースファイルの xc ディレクトリで次の
     手順を実行してください。

                gzip -d < R6.3pl1-3.3.diff1.gz | patch -p0 -E
                gzip -d < R6.3pl1-3.3.diff2.gz | patch -p0 -E
                gzip -d < R6.3pl1-3.3.diff3.gz | patch -p0 -E
                gzip -d < R6.3pl1-3.3.diff4.gz | patch -p0 -E
                gzip -d < cfont33.tgz | tar vxf -

     まっ更な変更していないソースファイルで行うことを確認してください。
     そうしないとパッチが失敗します。

  XFree86 X サーバだけを構築したいの時は、``servers only'' 配布物と呼ば
  れる XFree86 ソースファイルを切り詰めたものを使用しましょう。この方法
  を選択した場合は、次の手順を実行しましょう。:

  o  X33servonly.tgz というファイルを
     ftp://ftp.xfree86.org/pub/XFree86/3.3/source/ から (またはミラーサ
     イトの同様な場所から) 入手しましょう。

  o  次の手順でファイルを伸長してください。:

                gzip -d < X33servonly.tgz | tar vxf -

  XFree86 3.2 のソースから 3.3 に更新するパッチはありません。その理由は
  X11R6.1 から X11R6.3 に移行するにあたり関連する変更点が多すぎたためで
  す。

  XFree86 は X コンソーシアムの X11R6.1 contrib 配布物の一部をサポートし
  ています。これを構築したい場合は、次のファイルやディレクトリを配布物か
  ら入手しましょう。:

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

  また、XFree86 patch contrib-3.3.diff.gz が必要です。パッチを当てるため
  には、contrib ディレクトリがあるディレクトリで次のコマンドを実行してく
  ださい。:

                gzip -d < contrib-3.3.diff.gz | patch -p0 -E

  ほかの方法としては、XFree86 ソースディレクトリから X33contrib.tgz とい
  うファイルを入手して次のコマンドで伸長してください。:

                gzip -d < X33contrib.tgz | tar vxf -

  xtest 配布物を構築したい場合は、XFree86 ソースディレクトリから
  X33test.tgz ソース配布物 を入手して次のコマンドで伸長してください。:

                gzip -d < X33test.tgz | tar vxf -

  注意として、xtest は (X11R6.3 から) コア X11 配布物の一部ではなくなり
  ました。

  1.2.  構築前のソースの設定

  設定過程をはじめる前に xc/config/cf ディレクトリに移動して、
  xf86site.def ファイルを host.def にコピーしてください。そし
  て、host.def ファイルを通読いし、幾つかのパラメタを設定してください。
  良く読むことを強くお勧めします。標準値の設定は御使用の OS の名前に .cf
  が付いたファイルにありますので調べてください。

  前の版と異なっているのは、imake が自動認識するようになったことといろい
  ろな OS*Version パラメタを設定するようになったので、これらの設定を明示
  的に設定する必要がなくなりました。

  ソース配布物から X33src-1.tgz 部分だけを使用する場合は、 BuildFonts を
  NO にしてください。

  ``servers only'' 配布物を使用する場合は、 BuildServersOnly を YES にし
  てください。

  1.3.  配布物の構築と導入

  配布物を構築する前に、xc/programs/Xserver/hw/xfree86/doc にある適切な
  OS 固有の README ファイルを通読してください。 OS 固有の詳細な処理を
  行ってから、xc ディレクトリに移動して、 ``make World'' を OS 固有の
  README ファイルに従って必要に応じて BOOTSTRAPCFLAGS を追加して実行して
  ください。標準出力と標準エラー出力を World.Log にリダイレクトしておく
  と構築中に発生した問題を追跡するのに役に立ちます。

  構築が終了したら、 World.Log を見て問題が無いことを確認してください。
  問題がなければバイナリを導入できます。全てのソース配布物を使用している
  場合は、xc ディレクトリから導入を行います。``servers only'' 配布物を使
  用している場合は、xc/programs/Xserver ディレクトリから導入を行います。
  導入を行うには``make install'' と ``make install.man'' を実行してくだ
  さい。導入を行う /usr/X11R6 に十分な空き容量があることを確認してくださ
  い。/usr 以外のファイルシステムに導入したい場合は導入前に /usr/X11R6
  へシンボリックリンクを張ってください。

  /usr/X11R6/lib/Server にあるバイナリの LinkKit を導入するには、``make
  install.linkkit'' を xc ディレクトリから実行してください。

  XFree86 のサポートしている contrib の一部を構築するにはコア配布物が先
  ず導入してあることを確認してください。そうしたら contrib ディレクトリ
  に移動して ``xmkmf -a; make'' を実行してください。コンパイルが終了した
  ら ``make install'' と ``make install.man'' を実行して導入してくださ
  い。

  xtest 配布物を構築/実行するには、test/xsuite/NOTES.xf86 ファイルにある
  手順を参照してください。

  2.  サーバの再設定 (ソース配布物の場合)

  複数の異なるサーバや異なるドライバを含むサーバを構築するには:

  1. 新しいドライバが正しい位置 (例えば SVGA サーバのドライバのソースは
     xc/programs/Xserver/hw/xfree86/vga256/drivers というサブディレクト
     リに置いてあります) にあることを確認してください。

  2. サーバの定義の設定変更

  3. xc/programs/Xserver から実行してください。

                make Makefile
                make Makefiles
                make depend
                make

  3.  サーバの再設定 (バイナリ配布物の場合)

  サーバのバイナリリンクキットを導入する場合は、ドライバとサーバ内の幾つ
  かの機能拡張を再設定できます。この作業の詳細は README.LinkKit ファイル
  を参照して下さい。

  ______________________________________________________________________

  このファイルは xc/programs/Xserver/hw/xfree86/doc/sgml/BUILD.sgml,v
  3.1.2.4 1997/06/02 03:12:42 を、岡本  一幸 ( Kazuyuki Okamoto
  <ikko-@pacific.rim.or.jp> <mailto:ikko-@pacific.rim.or.jp> ) が
  XFree86 3.3 を日本でインストールする人向けに和訳したものです。ここがお
  かしいとか、ここはこうしたほうがいいといったご意見がありましたら、電子
  メールでお知らせ下さい。原文の著作権は XFree86 プロジェクト社にありま
  す。この和訳の著作権は XFree86 プロジェクト社と岡本 一幸にありますが、
  この和訳の不具合は私に、電子メールで送って下さい。

  $XFree86: xc/programs/Xserver/hw/xfree86/doc/Japanese/BUILD,v 1.1.2.1 1997/07/31 06:05:47 dawes Exp $

  ______________________________________________________________________

  [EOF]

