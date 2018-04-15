PROJECT = "SMS"
VERSION = "1.0.0"
require"sys"
require"common" --smsapp模块用到了common模块的接口
require"sms" --smsapp模块用到了sms模块的接口
require"smsapp"

sys.init(0,0)
sys.run()
