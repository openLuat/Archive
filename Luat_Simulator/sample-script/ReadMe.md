<html>
<head>
    <title>LuaT模拟器说明书</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
</head>
<body>
<h1>目录</h1>
<dl>
  <dt><a href="#title10">1. 安装JDK(Java SE Development Kit)</a></dt>
  <dt><a href="#title20">2. 下载LDT(Lua Development Tools)</a></dt>
  <dt><a href="#title30">3. 克隆LuaT模拟器</a></dt>
  <dt><a href="#title40">4. 安装C运行时库</a></dt>
  <dt><a href="#title50">5. 导入样例工程</a></dt>
  <dt><a href="#title60">6. 配置LDT的默认的Lua解释器</a></dt>
  <dt><a href="#title70">7. 开始运行/调试</a></dt>
</dl>

<hr/>

<a name="title10"></a>
<h2 id="title10" name="title10">1. 安装JDK(Java SE Development Kit)</h2>

<p>JDK(Java SE Development Kit)的官方下载地址：
<a href="http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html" >Java SE Development Kit 8u121</a></p>

<p>开始下载前必须接受Java SE的软件许可协议。 JDK 8U121的官方下载地址：</p>

<p>Windows X86 189.36MB <a href="http://download.oracle.com/otn-pub/java/jdk/8u121-b13/e9e7ea248e2c4826b92b3f075a80e441/jdk-8u121-windows-i586.exe" >jdk-8u121-windows-i586.exe</a></p>

<p>Windows X64 195.51MB <a href="http://download.oracle.com/otn-pub/java/jdk/8u121-b13/e9e7ea248e2c4826b92b3f075a80e441/jdk-8u121-windows-x64.exe" >jdk-8u121-windows-x64.exe</a></p>

<a name="title20"></a>
<h2 id="title20" name="title20">2. 下载LDT(Lua Development Tools)</h2>

<p>&emsp;&emsp;Lua开发工具（Lua Development Tools，LDT）一直以来是作为Eclipse的一个插件（Koneki）存在，现在LDT开发团队决定将其从Eclipse剥离，发展成为一个独立的开发环境。</p>

<p>&emsp;&emsp;LDT开发者Benjamin Cabé说，这个独立的预配置软件包，对于不是特别熟悉Eclipse SDK的开发者来说最好不过了。开发者可以在32位和64位的Windows、Mac OS X 和 Linux操作系统中使用它。</p>

<p>&emsp;&emsp;LDT基于Eclipse公共许可证。</p>

<p>LDT(Lua Development Tools)是一个免安装的Lua调试工具，解压后即可运行。在Win8/Win10系统运行时，必须赋予"LuaDevelopmentTools.exe"管理员权限。</p>

<a name="title30"></a>
<h2 id="title30" name="title30">3. 克隆LuaT模拟器</h2>
<p>克隆整个LuaT的开源模拟器，需用Git执行下述命令：</p>
<pre><code>
git clone git@github.com:airm2m-open/Luat_simulator.git
</pre></code>
<p>首次克隆LuaT模拟器需要花费较长时间，请耐心等待。若克隆失败，再次执行相同的克隆命令，直至克隆成功。</p>

<p>AMWatchDll：模拟器的中间件源码。</p>
<p>sample-script：一个由LDT创建的样例工程。</p>

<a name="title40"></a>
<h2 id="title40" name="title40">4. 安装C运行时库</h2>
<p>LuaT模拟器不支持Win7之前的系统，比如：Windows XP。要运行LuaT模拟器，必须安装C运行时库。对应的C运行时库已放入样例工程内：</p>

<p>Windows 32-bit，sample-script --> Win32_lib --> 'VC_x86Runtime.exe'</p>

<p>Windows 64-bit，sample-script --> x64_lib --> 'VC_x64Runtime.exe'</p>

<p>Win7/Win8系统已自带C运行时库，可不用安装C运行时库。</p>

<a name="title50"></a>
<h2 id="title50" name="title50">5. 导入样例工程</h2>
<p>导入样例工程前，必须创建一个自己的工作空间(Workspace)，并选择此工作空间作为调试Lua脚本的目录。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100045vs8ikza3z93iv9j9.png" alt="LDT-DOC-1">

<p>把sample-script样例工程导入上述的工作空间，且必须选中sample-script的根目录，否则无法引入正确的工程配置信息。</p>
<p>File --> Import... </p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100045csyjk6rkyg5jhtrk.png" alt="LDT-DOC-2">

<p> --> General --> Existing Projects into Workspace</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100046h2gkn6ivkdbdbhm4.png" alt="LDT-DOC-3">

<p> --> Next --> Select root directory</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100047tr1rsrshf01te19h.png" alt="LDT-DOC-4">

<p> --> Browse... </p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100047p77rz3r757ta797f.png" alt="LDT-DOC-5">

<p> --> OK </p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100048x0iloio2q64b6pfj.png" alt="LDT-DOC-6">

执行"Finish"后，即可成功导入sample-script。

<a name="title60"></a>
<h2 id="title60" name="title60">6. 配置LDT的默认的Lua解释器</h2>
<p>LuaT模拟器必须使用定制化的Lua解释器。</p>

<p>6.1 配置默认解释器</p>
<p>Window --> Preferences --> Lua --> Interpreters --> Add</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100048p33whrskhrrgo4ok.png" alt="LDT-DOC-7">

<p>Add interpreter --> Browse...</p>
<p>lua5.1.exe Lua脚本的解释器。</p>
<p>luac5.1.exe Lua脚本的编译器。</p>
<p>Windows 32-bit, sample-script --> Win32_lib --> Lua5.1 --> lua5.1.exe</p>

<p>Windows 64-bit, sample-script --> x64_lib --> Lua5.1 --> lua5.1.exe</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100049o7ask1aaa292299s.png" alt="LDT-DOC-8">

<p>可以给定制化的解释器取个别名(Interpreter name)，比如：AM Lua 5.1。执行"OK"。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100049zdylysplx1d5h5x0.png" alt="LDT-DOC-9">

<p>必须把上述定制化的Lua解释器，"AM Lua 5.1",置为选中状态。</p>

<p>6.2 配置运行时解释器</p>
<p>Run/Debug --> Run/Debug Configurations --> Runtime Interpreter</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100050p0pz6230ao239bq3.png" alt="LDT-DOC-10">

<p>按"New"按钮为Lua应用"Lua Application"创建一个新的Run/Debug配置。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100050u577hc2m3id33m2w.png" alt="LDT-DOC-11">

<p>启动脚本"Launch script"必须选择样例脚本"sample-script"下的"init.lua"，脚本"init.lua"为LuaT模拟器的入口。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100051vahib2w69ubozop0.png" alt="LDT-DOC-12">

<p>执行"OK"确定启动脚本的配置。运行解释器"Runtime Interpreter"必须配置为6.1中的默认解释器，"AM Lua 5.1"。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100051glmm7zftmj4u44t1.png" alt="LDT-DOC-13">

<p>执行"Apply"存储所有的配置。</p>

<a name="title70"></a>
<h2 id="title70" name="title70">7. 开始运行/调试</h2>
<p>资源文件 'sample-script\src\ldata'，比如： .mp3，.bmp，.png和.gif，把需要的资源文件拷贝到ldata目录。</p>
<p>Lua源文件 'sample-script\src\lua'，把工程源文件或库(lib)源文件拷贝到lua目录，也可在lua中创建新的Lua源文件。</p>
<p>日志文件 'sample-script\AMLuaDebug.log'，存储模拟器和Lua脚本的日志。</p>

<img src="http://bbs.airm2m.com/data/attachment/album/201702/14/100052cn09rfrmrun1g0gr.png" alt="LDT-DOC-14">

运行模拟器时必需选择6.2创建的Run配置："sample-script-init"。

<img src="http://bbs.airm2m.com/data/attachment/album/201703/03/111318xxd0spf94n99z0en.png" alt="LDT-DOC-27">

<img src="http://bbs.airm2m.com/data/attachment/album/201703/03/111318e8772w68w8y8yez8.png" alt="LDT-DOC-28">

<p>到目前为止，只能运行Run模式，Debug模式还无法正常运行。</p>

<p>
<a><b>注意事项：</a></b><br/>
1.用浏览器阅读md文件，必须安装对应的插件，否则无法查看。<br/>
2.建议使用MarkdownPad来阅读，而且必须破解，否则无法查看ReadMe.md中的图片。</br>
3.当前，用手机上的浏览器阅读本帖子无法查看图片。</br>
</p>

[Luat模拟器论坛](http://bbs.airm2m.com/forum.php?mod=forumdisplay&fid=81 "Luat模拟器论坛")

<div style="height:130px;">&nbsp;</div>
<div style="height:130px;">&nbsp;</div>
<div style="height:130px;">&nbsp;</div>

</body>
</html> 