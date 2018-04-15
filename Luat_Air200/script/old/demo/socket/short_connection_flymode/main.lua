PROJECT = "SOCKET_SHORT_CONNECTION_FLYMODE"
VERSION = "1.0.0"
require"sys"
--关闭脚本中的所有trace打印
--sys.opntrace(false)
require"link"
require"linkapp"
require"dbg"
dbg.setup("udp","www.test.com",9072)
require"update"
update.setup("udp","www.test.com",2233)
require"test"

net.setled(true)
sys.init(0,0)
--需要抓core中的trace时，打开如下三行
--ril.request("AT*TRACE=\"SXS\",1,0")
--ril.request("AT*TRACE=\"DSS\",1,0")
--ril.request("AT*TRACE=\"RDA\",1,0")
--设置工作模式为简单模式
sys.setworkmode(sys.SIMPLE_MODE)
sys.run()
