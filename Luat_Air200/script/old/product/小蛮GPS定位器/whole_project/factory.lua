--[[
模块名称：工程测试模块
模块功能：设备的功能测试：震动，gps，按键，充电状态，看门狗，网络，IMEI、SN的烧写及读取等
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

--ftrlt功能测试结果值
--fttimes功能测试时间值
--shkflag 震动标记，true：已震动，false或nil：未震动
--keyflag 按键标记，true：检测到按键按下，false或nil：未检测到按键按下
local ftrlt,fttimes,shkflag,keyflag = 0,0
local smatch,slen = string.match,string.len
local waitrst

--[[
函数名：rsp
功能  ：对收到的指令做出应答
参数  ：s，应答的内容
返回值：无
]]
local function rsp(s)
	print("factory rsp",s)
	uart.write(3,s)
end

--[[
函数名：timeout
功能  ：功能测试超时函数
参数  ：无
返回值：无
]]
local function timeout()
	local s1
	fttimes = fttimes + 1
	print("factory timeout",fttimes)
    --功能测试时间超过20秒也必须做出应答
	if fttimes > 20 then
	    s1 = "\r\n" .. "+FT5315:" .. "\r\n" .. ftrlt .. "\r\n" .. "OK" .. "\r\n"
		rsp(s1)
		return
	end
    --仍有测试项不通过，继续下一轮的检测
	checkft()
end

--[[
函数名：checkft
功能  ：检测功能测试结果
参数  ：无
返回值：无
]]
function checkft()
	print("factory checkft",net.getstate(),net.getrssi(),gps.getgpssn(),shkflag,chg.getcharger(),chg.getstate(),keyflag)
	local s1
    --如果网络注册成功，且rssi值大于15，则认为GSM测试项通过，如果ftrlt第0位为0，则将第0位置1
	if net.getstate()=="REGISTERED" and net.getrssi() > 15 and (ftrlt % 2) == 0 then
		ftrlt = ftrlt + 1
	end
    --如果gps信噪比值大于30，则认为GPS测试项通过，如果ftrlt第1位为0，则将第1位置1
	if gps.getgpssn() > 30 then
		if (ftrlt % 4) < 2 then
			ftrlt = ftrlt + 2
		end
	end
    --如果有震动上报且ftrlt第2位位0，则将第2位置1
	if shkflag then
		if (ftrlt % 8) < 4 then
			ftrlt = ftrlt + 4
		end
	end
	--if acc.getflag() then
		if (ftrlt % 16) < 8 then
			ftrlt = ftrlt + 8
		end
	--end
    --如果充电器在位且ftrlt第4位位0，则将第4位置1
	if chg.getcharger() then
		if (ftrlt % 32) < 16 then
			ftrlt = ftrlt + 16
		end
	end
    --如果充电器在充电或充满且ftrlt第5位位0，则将第5位置1
	if chg.getstate()~=0 then
		if (ftrlt % 64) < 32 then
			ftrlt = ftrlt + 32
		end
	end
    --如果检测到按键上报且ftrlt第6位位0，则将第6位置1
	if keyflag then
		if (ftrlt % 128) < 64 then
			ftrlt = ftrlt + 64
		end
	end

    --如果测试不通过，10秒后再次检查测试结果，如果测试项全通过，直接向工具做出应答，
	if ftrlt < 127 then
		print("factory timeout",fttimes)
		sys.timer_start(timeout,1000)
	else		
		s1 = "\r\n" .. "+FT5315:" .. "\r\n" .. ftrlt .. "\r\n" .. "OK" .. "\r\n"
		rsp(s1)
	end
end

--[[
函数名：cb
功能  ：回调函数
参数  ：cmd,success,response,intermediate
返回值：无
]]
local function cb(cmd,success,response,intermediate)
	if (smatch(cmd,"WIMEI=") or smatch(cmd,"WISN=") or smatch(cmd,"AMFAC=")) and success then
		rsp("\r\nOK\r\n")
	end
end

--[[
函数名：proc
功能  ：串口收到的数据的处理函数
参数  ：s 串口收到的数据
返回值：无
]]
local function proc(s)
	s = string.upper(s)
	if smatch(s,"WIMEI=") then
		misc.set("WIMEI",smatch(s,"=\"(.+)\""),cb)
		waitrst = true
	elseif smatch(s,"CGSN") then
		if waitrst then
			rsp("\r\nAT+CGSN\r\nWAIT RST\r\nOK\r\n")
		else
			local imei = misc.getimei()
			if imei and slen(imei) > 0 then
				rsp("\r\nAT+CGSN\r\n" .. imei .. "\r\nOK\r\n")
			end
		end
	elseif smatch(s,"WISN=") then
		misc.set("WISN",smatch(s,"=\"(.+)\""),cb)
		waitrst = true
	elseif smatch(s,"WISN%?") then
		if waitrst then
			rsp("\r\nAT+WISN?\r\nWAIT RST\r\nOK\r\n")
		else
			local sn = misc.getsn()
			if sn and slen(sn) > 0 then
				rsp("\r\nAT+WISN?\r\n" .. sn .. "\r\nOK\r\n")
			end
		end
	elseif smatch(s,"VER") then
		rsp("\r\nAT+VER\r\n" .. _G.PROJECT .. "_" .. _G.VERSION .."\r\nOK\r\n")
	elseif smatch(s,"CHARGER?") then
		rsp("\r\nAT+CHARGER?\r\n" .. (chg.getcharger() and 1 or 0) .."\r\nOK\r\n")
	elseif smatch(s,"AMFAC=") then
		gpsapp.open(gpsapp.OPEN_DEFAULT,{cause="FAC"})
		misc.set("AMFAC",smatch(s,"=(.+)"),cb)
	elseif smatch(s,"CFUN=") then
		--gpsapp.close(gpsapp.OPEN_DEFAULT,{cause="FAC"})
		--misc.set("CFUN",smatch(s,"=(.+)"))
		waitrst = true
		--uart.close(3)
		rtos.restart()
	elseif smatch(s,"WDTEST") then
		rsp("\r\nAT+WDTEST\r\nOK\r\n")
		wdt153b.test()
	elseif smatch(s,"AT%+FT5315") then
		sys.dispatch("FAC_IND")
		gpsapp.open(gpsapp.OPEN_DEFAULT,{cause="FAC"})
		checkft()
	end
end

--[[
函数名：read
功能  ：串口接收消息函数
参数  ：无
返回值：无
]]
local function read()
	local t1
	local s1 = ""
	local rd = true
	while rd == true do
		t1 = uart.read(3,"*l",0)
		if t1 and t1~="" then
			s1 = s1 .. t1
		else
			rd = false
		end		
	end

	if s1 ~= "" then
		print("factory proc:",s1)
	end
    --处理串口收到的数据
	proc(s1)
end

--[[
函数名：ind
功能  ：震动，按键消息处理函数
参数  ：id，消息id
         data， 收到的数据
返回值：无
]]
local function ind(id,data)
	if id == "DEV_SHK_IND" then
		shkflag = true
	elseif id=="MMI_KEYPAD_IND" then
		keyflag= true
	end
	return true
end

--设置串口，用于与工程测试的工具通信
uart.setup(3,921600,8,uart.PAR_NONE,uart.STOP_1,2)
--注册串口的接收消息函数
sys.reguart(3,read)
net.startquerytimer()
--注册震动，按键消息处理函数
sys.regapp(ind,"DEV_SHK_IND","MMI_KEYPAD_IND")
