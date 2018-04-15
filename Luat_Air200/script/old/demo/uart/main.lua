PROJECT = "UART"
VERSION = "1.0.0"
require"sys"
require"common" --test模块用到了common.binstohexs接口
require"pm" --test模块用到了pm.wake接口
require"test"

sys.init(0,0)
sys.run()
