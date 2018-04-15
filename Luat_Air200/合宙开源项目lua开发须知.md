### 合宙开源项目lua开发一般步骤
如果用户手上已经有合宙Air200模块开发板或小蛮GPS定位器，此时就具备实体开发调试的基本条件了

#### 第一步:clone 合宙开源库luat到本地电脑
可以从github克隆，也可以从百度网盘，开源社区下载。  

开源资料包架构如下：  

\core是模块基础软件代码，支持Lua的运行。core代码编译生成的文件叫做lod文件。我司已经提供了几个lod文件，放置在core根目录。core内含合宙自己开发的Lua扩展库（位于\core\cust_src\elua\modules\src），扩展库文档位于\doc目录中。

\script是Luat范例脚本，其中：  
\xiaoman_gps_tracker\whole_test 是一款已经商用的GPS定位器软件，可以下到合宙小蛮GPS定位器（是基于Air200开发的）中运行。

\demo下timer、 UART 等是各种应用例程，可下载到合宙EVB板中运行。  合宙EVB开发板对应的硬件参考和Air200相关的AT命令位于\doc中。 

\lib 是demo，product以及所有用户脚本都需要用到的库文件，这些库文件将经常使用的AT命令以函数形式封装，方便用户使用。

\tools目录下放了一些用户开发过程需要用到的工具。其中：  

LuaForWindows 是一款Lua代码编辑和语法检查工具。它的使用方法请参考附带的安装使用说明以及\doc\合宙通信OpenLuat开源模块LUA程序设计指南 这篇文档。  

LuaDB合并及下载工具是下载lod文件和用户Lua脚本到模块的工具软件。
 
RDA平台trace工具是一款轻型的trace打印工具。

Luat IDE是Luat专用的集成开发环境，集编辑、调试、下载和trace功能于一体。

#### 第二步：用户编写Lua代码

用户对Lua项目的二次开发，请参考\doc 下Luat 脚本开发指南 这篇文档。  
另外lib库文件和demo文件都给出了sample代码和详细注释。  
编辑工具可使用我司提供的LuaForWindows。

LuaForWindows下载地址：

http://bbs.openluat.com/forum.php?mod=viewthread&tid=2415&extra=page%3D1

#### 第三步：LUA代码和lod，合并下载到合宙模块中
用户自己开发的LUA代码和lod，需要一起下载到合宙模块中。
需使用 LuaDB合并及下载工具 进行下载。该工具附带有一个简要的工具使用说明。  
所用的lod文件请直接使用\core下的lod文件。  

LuaDB下载工具下载地址：

http://bbs.openluat.com/forum.php?mod=viewthread&tid=2601&extra=page%3D1

### 第四步：查看模块trace打印
用户需要查看模块的trace以检查和修改自己的Lua代码。

用户可以使用合宙发布的一款轻型trace打印工具：RDA平台trace工具 来查看trace。 
 
RDA平台trace工具下载地址：

http://bbs.openluat.com/forum.php?mod=viewthread&tid=2601&extra=page%3D1