<!-- $XFree86: xc/programs/Xserver/hw/darwin/bundle/Japanese.lproj/XDarwinHelp.html.cpp,v 1.1.2.2 2001/05/22 03:36:53 torrey Exp $ -->

#include "xf86Version.h"
#ifndef PRE_RELEASE
#define PRE_RELEASE XF86_VERSION_SNAP
#endif

<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=EUC-JP">
<title>
XFree86 for Mac OS X</title></head>
<body>
<center>
    <h1>XFree86 on Darwin and Mac OS X</h1>
    XFree86 XF86_VERSION<br>
    Release Date: XF86_REL_DATE
</center>
<h2>目次</h2>
<ol>
    <li><A HREF="#notice">注意事項</A></li>
    <li><A HREF="#usage">使用法</A></li>
    <li><A HREF="#path">パスの設定</A></li>
    <li><A HREF="#prefs">環境設定</A></li>
    <li><A HREF="#license">ライセンス</A></li>
</ol>
<center>
        <h2><a NAME="notice">注意事項</a></h2>
</center>
<blockquote>
#if PRE_RELEASE
これは，XFree86 のプレリリースバージョンであり，いかなる場合においてもサポートされません。 
バグの報告やパッチが SourceForge の <A HREF="http://sourceforge.net/projects/xonx/">XonX プロジェクトページ</A>に提出されているかもしれません。
プレリリースバージョンのバグを報告する前に，<A HREF="http://sourceforge.net/projects/xonx/">XonX</A> プロジェクトページまたは <A HREF="http://www.XFree86.Org/cvs">XFree86 CVS リポジトリ</A>で最新版のチェックをして下さい。
</blockquote>
#else
もし，サーバーが 6 -12 ヶ月以上前のものか，またはあなたのハードウェアが上記の日付よりも新しいものならば，問題を報告する前により新しいバージョンを探してみてください。
バグの報告やパッチが SourceForge の <A HREF="http://sourceforge.net/projects/xonx/">XonX プロジェクトページ</A>に提出されているかもしれません。
#endif
</blockquote>
<blockquote>
本ソフトウェアは，<A HREF="#license">MIT X11/X Consortium License</A> の条件に基づき，無保証で，「そのまま」の形で供給されます。
ご使用になる前に，<A HREF="#license">ライセンス条件</A>をお読み下さい。
</blockquote>

<h2><a NAME="usage">使用法</a></h2>
<p>
XFree86 は，<a HREF="http://www.XFree86.Org/">XFree86 Project, Inc.</a>によって作成された，再配布可能なオープンソースの <a HREF="http://www.x.org/">X Window System</a> の実装です。
Mac OS X 上で，XFree86 はフルスクリーンモードで動作します。X Window System がアクティブな時，それは全画面を占有します。
あなたは，Command-Option-A キーを押すことによって Mac OS X デスクトップへ切り替えることができます。このキーの組み合わせは，環境設定で変更可能です。
Mac OS X デスクトップからX Window System へ切り替える場合は，フローティング・ウィンドウに表示された XDarwin アイコンをクリックしてください。
ドックに表示された XDarwin アイコンのクリックで X Window System へ切り替わるように，環境設定で動作を変更することもできます。
</p>

<h3>複数ボタンマウスのエミュレーション</h3>
<p>
多くの X11 アプリケーションは，3 ボタンマウスを必要とします。
1 ボタンマウスで 3 ボタンマウスをエミュレーションするには，環境設定で「複数ボタンマウスのエミュレーションを有効にする」を選択します。
これは，次回の X サーバーの起動時より有効となります。

3 ボタンマウスをエミュレーションする時，左のコマンドキーを押しながらマウスボタンをクリックすることは第 2 マウスボタンのクリックに相当します。左のオプションキーを押しながらクリックすることは第 3 マウスボタンのクリックに相当します。
</p>
<p>注：</p>
<ul>
    <li>多くのキーボードでは，左右のコマンドキーとオプションキーは区別されず，同じ動作をします。
    <li>xmodmap でコマンドキーやオプションキーを他のキーに割り当てている場合でも，複数ボタンマウスのエミュレーションでは本来のコマンドキーやオプションキーを使わなければなりません。
    <li>左のコマンドキーを押しながら第 2 マウスボタンをクリックすることを実現するには，左のコマンドキーを他のキーに割り当てる必要があります。左のオプションキーを押しながら第 3 マウスボタンをクリックする場合も同様です。
</ul>
<h2>
<a NAME="path">パスの設定</a>
</h2>
<p>
X11 バイナリは，/usr/X11R6/bin に置かれます。あなたはそれをパスに加える必要があるかもしれません。
パスは， 実行可能なコマンドを検索するディレクトリのリストです。
これを確認する方法は，使用しているシェルに依存します。
tcsh では，「printenv PATH」とタイプすることでパスをチェックすることができます。
パスをチェックした時，/usr/X11R6/bin がディレクトリのひとつとして表示されなければなりません。
もしそうでなければ，あなたはそれをデフォルトのパスに追加する必要があります。
そのために，~/Library/init/tcsh/path ファイルに次の行を追加して下さい。
（もしこのファイルが無ければ作成して下さい）
</p>
<blockquote>
setenv PATH "${PATH}:/usr/X11R6/bin"
</blockquote>
<p>
もし，あなたが .cshrc ファイルまたは .tcshrc ファイルを作成するならば，それらのファイルは ~/Library/init/tcsh/path ファイルの設定値を上書きします。
そして，あなたはそれらのファイルのうちの一つだけを変更する必要があることに注意して下さい。
これらの変更は，あなたが新たな Terminal ウィンドウを開始するまで有効となりません。
また，あなたはドキュメントを探している時，XFree86 のマニュアルページを検索されるページのリストに追加したいと思うかもしれません。
X11 のマニュアルページは /usr/X11R6/man に置かれます。そして MANPATH 環境変数は検索するディレクトリのリストを含んでいます。
</p>

<h2><a NAME="prefs">環境設定</a></h2>
<p>「XDarwin」メニューの「環境設定...」メニュー項目から，いくつかの設定を行うことができます。 
「起動オプション」の内容は，XDarwin を再起動するまで有効となりません。
他の全てのオプションの内容は，直ちに有効となります。
以下，それぞれのオプションについて説明します:</p>
<ul>
    <li>キー設定ボタン: X11 と Aqua を切り替えるためのキーの組み合わせを変更するために，ボタンをクリックして，いくつかの修飾キーに続いて通常のキーを押します。</li>
    <li>X11 でシステムのビープ音を使用する: オンの場合，Mac OS X のビープ音が X11 のベルとして使用されます。オフの場合（デフォルト），シンプル トーンが使われます。</li>
    <li>ドックのアイコンのクリックで X11 に戻る: オンの場合，ドックに表示された XDarwin アイコンのクリックで X11 への切り替えが可能となります。
    Mac OS X のバージョンによっては，ドックのアイコンのクリックで画面を切り替えると，Aqua に戻った時にカーソルが消失することがあります。</li>
    <li>起動時にヘルプを表示する: XDarwin の起動時に，スプラッシュ スクリーンを表示します。</li>
    <li>ディスプレイ番号:  XDarwin がディスプレイに割り当てる X の Display Number を指定します。 
    X11 を表示するとき，XDarwin は常にメインディスプレイを引き継ぐことに注意して下さい。</li>
    <li>キーマッピング: デフォルトでは，XDarwin は Darwin カーネルからキーマッピングをロードします。
        ポータブル機種では，時々キーマッピングが空となり，X11 でキーボードが動作しなくなります。
       「ファイルからロードする」を選択すると，XDarwin は指定されたファイルからキーマッピングをロードします。<br>
    （訳注：キーマッピングで Japanese を選択すると，不具合が生じます。USA を選択した上で ~/.Xmodmap を適用して下さい。）</li>
</ul>

<h2>
<a NAME="license">ライセンス</a>
</h2>
XFree86 Project は，自由に再配布可能なバイナリとソースコードを提供することにコミットしています。
私たちが使用する主なライセンスは，伝統的な MIT X11/X Consortium License に基づくものです。
そして，それは修正または再配布されるソースコードまたはバイナリに，その Copyright/ライセンス告示がそのまま残されることを要求する以外の条件を強制しません。
より多くの情報と，コードの一部をカバーする追加の Copyright/ライセンス告示のために，<A HREF="http://www.xfree86.org/legal/licence.html">XFree86 の License ページ</A>を参照して下さい。
<H3>
<A NAME="3"></A>
X  Consortium License</H3>
<p>Copyright (C) 1996 X Consortium</p>
<p>Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without
limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to
whom the Software is furnished to do so, subject to the following conditions:</p>
<p>The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.</p>
<p>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.</p>
<p>Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization from
the X Consortium.</p>
<p>X Window System is a trademark of X Consortium, Inc.</p>
</body>
</html>
