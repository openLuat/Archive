--[[
模块名称：短信管理
模块功能：短信发送、短信接收、短信读取、短信删除
模块最后修改时间：2017.02.20
]]
module(...,package.seeall)

--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上smsapp前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("smsapp",...)
end

-----------------------------------------短信发送功能封装[开始]-----------------------------------------
--短信发送缓冲表最大个数
local SMS_SEND_BUF_MAX_CNT = 10
--短信发送间隔，单位毫秒
local SMS_SEND_INTERVAL = 3000
--短信发送缓冲表
local tsmsnd = {}

--[[
函数名：sndnxt
功能  ：发送短信发送缓冲表中的第一条短信
参数  ：无
返回值：无
]]
local function sndnxt()
	if #tsmsnd>0 then
		sms.send(tsmsnd[1].num,tsmsnd[1].data)
	end
end

--[[
函数名：sendcnf
功能  ：SMS_SEND_CNF消息的处理函数，异步通知短信发送结果
参数  ：
        result：短信发送结果，true为成功，false或者nil为失败
返回值：无
]]
local function sendcnf(result)
	print("sendcnf",result)
	local num,data,cb = tsmsnd[1].num,tsmsnd[1].data,tsmsnd[1].cb
	--从短信发送缓冲表中移除当前短信
	table.remove(tsmsnd,1)
	--如果有发送回调函数，执行回调
	if cb then cb(result,num,data) end
	--如果短信发送缓冲表中还有短信，则SMS_SEND_INTERVAL毫秒后，继续发送下条短信
	if #tsmsnd>0 then sys.timer_start(sndnxt,SMS_SEND_INTERVAL) end
end

--[[
函数名：send
功能  ：发送短信
参数  ：
        num：短信接收方号码，ASCII码字符串格式
		data：短信内容，UCS2大端格式的16进制字符串
		cb：短信发送结果异步返回时使用的回调函数，可选
		idx：插入短信发送缓冲表的位置，可选，默认是插入末尾
返回值：返回true，表示调用接口成功（并不是短信发送成功，短信发送结果，通过sendcnf返回，如果有cb，会通知cb函数）；返回false，表示调用接口失败
]]
function send(num,data,cb,idx)
	--号码或者内容非法
	if not num or num=="" or not data or data=="" then return end
	--短信发送缓冲表已满
	if #tsmsnd>=SMS_SEND_BUF_MAX_CNT then return end
	--如果指定了插入位置
	if idx then
		table.insert(tsmsnd,idx,{num=num,data=data,cb=cb})
	--没有指定插入位置，插入到末尾
	else
		table.insert(tsmsnd,{num=num,data=data,cb=cb})
	end
	--如果短信发送缓冲表中只有一条短信，立即触发短信发送动作
	if #tsmsnd==1 then sms.send(num,data) return true end
end
-----------------------------------------短信发送功能封装[结束]-----------------------------------------



-----------------------------------------短信接收功能封装[开始]-----------------------------------------
local function handle(num,data,datetime)
	print("handle",num,data,datetime)
	--回复相同内容的短信到发送方
	--if num then send(num,common.binstohexs(common.gb2312toucs2be(data))) end
end

--短信接收位置表
local tnewsms = {}

--[[
函数名：readsms
功能  ：读取短信接收位置表中的第一条短信
参数  ：无
返回值：无
]]
local function readsms()
	if #tnewsms ~= 0 then
		sms.read(tnewsms[1])
	end
end

--[[
函数名：newsms
功能  ：SMS_NEW_MSG_IND（未读短信或者新短信主动上报的消息）消息的处理函数
参数  ：
        pos：短信存储位置
返回值：无
]]
local function newsms(pos)
	--存储位置插入到短信接收位置表中
	table.insert(tnewsms,pos)
	--如果只有一条短信，则立即读取
	if #tnewsms == 1 then
		readsms()
	end
end

--[[
函数名：readcnf
功能  ：SMS_READ_CNF消息的处理函数，异步返回读取的短信内容
参数  ：
        result：短信读取结果，true为成功，false或者nil为失败
		num：短信号码，ASCII码字符串格式
		data：短信内容，UCS2大端格式的16进制字符串
		pos：短信的存储位置，暂时没用
		datetime：短信日期和时间，ASCII码字符串格式
		name：短信号码对应的联系人姓名，暂时没用
返回值：无
]]
local function readcnf(result,num,data,pos,datetime,name)
	--过滤号码中的86和+86
	local d1,d2 = string.find(num,"^([%+]*86)")
	if d1 and d2 then
		num = string.sub(num,d2+1,-1)
	end
	--删除短信
	sms.delete(tnewsms[1])
	--从短信接收位置表中删除此短信的位置
	table.remove(tnewsms,1)
	if data then
		--短信内容转换为GB2312字符串格式
		data = common.ucs2betogb2312(common.hexstobins(data))
		--用户应用程序处理短信
		handle(num,data,datetime)
	end
	--继续读取下一条短信
	readsms()
end
-----------------------------------------短信接收功能封装[结束]-----------------------------------------

--短信模块的内部消息处理表
local smsapp =
{
	SMS_NEW_MSG_IND = newsms, --收到新短信，sms.lua会抛出SMS_NEW_MSG_IND消息
	SMS_READ_CNF = readcnf, --调用sms.read读取短信之后，sms.lua会抛出SMS_READ_CNF消息
	SMS_SEND_CNF = sendcnf, --调用sms.send发送短信之后，sms.lua会抛出SMS_SEND_CNF消息
	SMS_READY = sndnxt, --底层短信模块准备就绪
}

--注册消息处理函数
sys.regapp(smsapp)



-----------------------------------------短信发送测试[开始]-----------------------------------------
local function sendtest1(result,num,data)
	print("sendtest1",result,num,data)
end

local function sendtest2(result,num,data)
	print("sendtest2",result,num,data)
end

local function sendtest3(result,num,data)
	print("sendtest3",result,num,data)
end

local function sendtest4(result,num,data)
	print("sendtest4",result,num,data)
end

send("10086",common.binstohexs(common.gb2312toucs2be("111111")),sendtest1)
send("10086",common.binstohexs(common.gb2312toucs2be("第2条短信")),sendtest2)
send("10086",common.binstohexs(common.gb2312toucs2be("qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432")),sendtest3)
send("10086",common.binstohexs(common.gb2312toucs2be("华康是的撒qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432qeiuqwdsahdkjahdkjahdkja122136489759725923759823hfdskfdkjnbzndkjhfskjdfkjdshfkjdsfks83478648732432")),sendtest4)
-----------------------------------------短信发送测试[结束]-----------------------------------------
