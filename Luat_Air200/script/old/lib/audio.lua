--[[
模块名称：音频控制
模块功能：dtmf编解码、tts（需要底层软件支持）、音频文件的播放和停止、录音、mic和speaker的控制
模块最后修改时间：2017.02.20
]]

--定义模块,导入依赖库
local base = _G
local string = require"string"
local io = require"io"
local rtos = require"rtos"
local audio = require"audiocore"
local sys = require"sys"
local ril = require"ril"
module("audio")

--加载常用的全局函数至本地
local smatch = string.match
local print = base.print
local dispatch = sys.dispatch
local req = ril.request
local tonumber = base.tonumber

--speakervol：speaker音量等级，取值范围为audio.VOL0到audio.VOL7，audio.VOL0为静音
--audiochannel：音频通道，跟硬件设计有关，用户程序需要根据硬件配置
--microphonevol：mic音量等级，取值范围为audio.MIC_VOL0到audio.MIC_VOL15，audio.MIC_VOL0为静音
local speakervol,audiochannel,microphonevol = audio.VOL4,audio.HANDSET,audio.MIC_VOL15
-- GSM全速率模式
local gsmfr = false 
local ttscause
--音频文件路径
local playname

--[[
函数名：setGSMFR
功能  ：设置GSM全速率（目前功能无效）
参数  ：无		
返回值：无
]]
function setGSMFR()
	gsmfr = true
	req([[AT+SCFG="Call/SpeechVersion1",2]])
end

--[[
函数名：dtmfdetect
功能  ：设置dtmf检测是否使能以及灵敏度
参数  ：
		enable：true使能，false或者nil为不使能
		sens：灵敏度，默认3，最灵敏为1
返回值：无
]]
function dtmfdetect(enable,sens)
	if enable == true then
		if not gsmfr then setGSMFR() end

		if sens then
			req("AT+DTMFDET=2,1," .. sens)
		else
			req("AT+DTMFDET=2,1,3")
		end
	end

	req("AT+DTMFDET="..(enable and 1 or 0))
end

--[[
函数名：senddtmf
功能  ：发送dtmf到对端
参数  ：
		str：dtmf字符串
		playtime：每个dtmf播放时间，单位毫秒，默认100
		intvl：两个dtmf间隔，单位毫秒，默认100
返回值：无
]]
function senddtmf(str,playtime,intvl)
	if string.match(str,"([%dABCD%*#]+)") ~= str then
		print("senddtmf: illegal string "..str)
		return false
	end

	playtime = playtime and playtime or 100
	intvl = intvl and intvl or 100

	req("AT+SENDSOUND="..string.format("\"%s\",%d,%d",str,playtime,intvl))
end

--[[
函数名：playtts
功能  ：播放tts
参数  ：
		text：字符串
		path："net"表示网络播放，其余值表示本地播放
返回值：true
]]
function playtts(text,path)
	local action = path == "net" and 4 or 2

	req("AT+QTTS=1")
	req(string.format("AT+QTTS=%d,\"%s\"",action,text))
	return true
end

--[[
函数名：stoptts
功能  ：停止播放tts
参数  ：无
返回值：无
]]
function stoptts()
	req("AT+QTTS=3")
end

--[[
函数名：closetts
功能  ：关闭tts功能
参数  ：
		cause：关闭原因
返回值：无
]]
function closetts(cause)
	ttscause = cause
	req("AT+QTTS=0")
end

--[[
函数名：transvoice
功能  ：通话中发送声音到对端,必须是12.2K AMR格式
参数  ：
返回值：true为成功，false为失败
]]
function transvoice(data,loop,loop2)
	local f = io.open("/RecDir/rec000","wb")

	if f == nil then
		print("transvoice:open file error")
		return false
	end

	-- 有文件头并且是12.2K帧
	if string.sub(data,1,7) == "#!AMR\010\060" then
	-- 无文件头且是12.2K帧
	elseif string.byte(data,1) == 0x3C then
		f:write("#!AMR\010")
	else
		print("transvoice:must be 12.2K AMR")
		return false
	end

	f:write(data)
	f:close()

	req(string.format("AT+AUDREC=%d,%d,2,0,50000",loop2 == true and 1 or 0,loop == true and 1 or 0))

	return true
end

--[[
函数名：beginrecord
功能  ：开始录音
参数  ：
		id：录音id，会根据这个id存储录音文件，取值范围0-4
		duration：录音时长，单位毫秒
返回值：true
]]
function beginrecord(id,duration)
	req(string.format("AT+AUDREC=0,0,1," .. id .. "," .. duration))
	return true
end

--[[
函数名：endrecord
功能  ：结束录音
参数  ：
		id：录音id，会根据这个id存储录音文件，取值范围0-4
		duration：录音时长，单位毫秒
返回值：true
]]
function endrecord(id,duration)
	req(string.format("AT+AUDREC=0,0,0," .. id .. "," .. duration))
	return true
end

--[[
函数名：delrecord
功能  ：删除录音文件
参数  ：
		id：录音id，会根据这个id存储录音文件，取值范围0-4
		duration：录音时长，单位毫秒
返回值：true
]]
function delrecord(id,duration)
	req(string.format("AT+AUDREC=0,0,4," .. id .. "," .. duration))
	return true
end

--[[
函数名：playrecord
功能  ：播放录音文件
参数  ：
		dl：模块下行（耳机或手柄或喇叭）是否可以听到录音播放的声音，true可以听到，false或者nil听不到
		loop：是否循环播放，true为循环，false或者nil为不循环
		id：录音id，会根据这个id存储录音文件，取值范围0-4
		duration：录音时长，单位毫秒
返回值：true
]]
function playrecord(dl,loop,id,duration)
	req(string.format("AT+AUDREC=" .. (dl and 1 or 0) .. "," .. (loop and 1 or 0) .. ",2," .. id .. "," .. duration))
	return true
end

--[[
函数名：stoprecord
功能  ：停止播放录音文件
参数  ：
		dl：模块下行（耳机或手柄或喇叭）是否可以听到录音播放的声音，true可以听到，false或者nil听不到
		loop：是否循环播放，true为循环，false或者nil为不循环
		id：录音id，会根据这个id存储录音文件，取值范围0-4
		duration：录音时长，单位毫秒
返回值：true
]]
function stoprecord(dl,loop,id,duration)
	req(string.format("AT+AUDREC=" .. (dl and 1 or 0) .. "," .. (loop and 1 or 0) .. ",3," .. id .. "," .. duration))
	return true
end

--[[
function playamfgp(namepath,typ)
	req(string.format("AT+AMFGP=1,\"".. namepath .. "\"," .. (typ and typ or 1)))
	return true
end

function stopamfgp(namepath,typ)
	req(string.format("AT+AMFGP=0,\"".. namepath .. "\"," .. (typ and typ or 1)))
	return true
end
]]

--[[
函数名：play
功能  ：播放音频文件
参数  ：
		name：音频文件路径
		loop：是否循环播放，true为循环，false或者nil为不循环
返回值：调用播放接口是否成功，true为成功，false为失败
]]
function play(name,loop)
	if loop then playname = name end
	return audio.play(name)
end

--[[
函数名：stop
功能  ：停止播放音频文件
参数  ：无
返回值：调用停止播放接口是否成功，true为成功，false为失败
]]
function stop()
	playname = nil
	return audio.stop()
end

local dtmfnum = {[71] = "Hz1000",[69] = "Hz1400",[70] = "Hz2300"}

--[[
函数名：parsedtmfnum
功能  ：dtmf解码，解码后，会产生一个内部消息AUDIO_DTMF_DETECT，携带解码后的DTMF字符
参数  ：
		data：dtmf字符串数据
返回值：无
]]
local function parsedtmfnum(data)
	local n = base.tonumber(string.match(data,"(%d+)"))
	local dtmf

	if (n >= 48 and n <= 57) or (n >=65 and n <= 68) or n == 42 or n == 35 then
		dtmf = string.char(n)
	else
		dtmf = dtmfnum[n]
	end

	if dtmf then
		dispatch("AUDIO_DTMF_DETECT",dtmf)
	end
end

--[[
函数名：audiourc
功能  ：本功能模块内“注册的底层core通过虚拟串口主动上报的通知”的处理
参数  ：
		data：通知的完整字符串信息
		prefix：通知的前缀
返回值：无
]]
local function audiourc(data,prefix)
	--DTMF接收检测
	if prefix == "+DTMFDET" then
		parsedtmfnum(data)
	--录音或者录音播放功能
	elseif prefix == "+AUDREC" then
		local action,duration = string.match(data,"(%d),(%d+)")
		if action and duration then
			duration = base.tonumber(duration)
			--开始录音
			if action == "1" then
				dispatch("AUDIO_RECORD_IND",(duration > 0 and true or false),duration)
			--播放录音
			elseif action == "2" then
				if duration > 0 then
					dispatch("AUDIO_PLAY_END_IND")
				else
					dispatch("AUDIO_PLAY_ERROR_IND")
				end
			--删除录音
			elseif action == "4" then
				dispatch("AUDIO_RECORD_IND",true,duration)
			end
		end
	--tts功能
	elseif prefix == "+QTTS" then
		local flag = string.match(data,": *(%d)",string.len(prefix)+1)
		--停止播放tts
		if flag == "0" then
			dispatch("AUDIO_PLAY_END_IND")
		end
	--[[elseif prefix == "+AMFGP" then
		local action = string.match(data,": *(%d)",string.len(prefix)+1)
		if action then
			if action == "0" then
				dispatch("AUDIO_PLAY_END_IND")
			elseif action == "1" then
				dispatch("AUDIO_PLAY_ERROR_IND")
			end
		end]]
	end
end

--[[
函数名：audiorsp
功能  ：本功能模块内“通过虚拟串口发送到底层core软件的AT命令”的应答处理
参数  ：
		cmd：此应答对应的AT命令
		success：AT命令执行结果，true或者false
		response：AT命令的应答中的执行结果字符串
		intermediate：AT命令的应答中的中间信息
返回值：无
]]
local function audiorsp(cmd,success,response,intermediate)
	local prefix = smatch(cmd,"AT(%+%u+%?*)")

	--录音或者播放录音确认应答
	if prefix == "+AUDREC" then
		dispatch("AUDIO_RECORD_CNF",success)
	--播放tts或者关闭tts应答
	elseif prefix == "+QTTS" then
		local action = smatch(cmd,"QTTS=(%d)")
		if not success then
			if action == "1" or action == "2" then
				--播放失败，产生一个内部消息
				dispatch("AUDIO_PLAY_ERROR_IND")
			end
		else
			if action == "0" then
				dispatch("TTS_CLOSE_IND",ttscause)
			end
		end
		if action=="3" then
			dispatch("TTS_STOP_IND")
		end
	end
end

--注册以下通知的处理函数
ril.regurc("+DTMFDET",audiourc)
ril.regurc("+AUDREC",audiourc)
--ril.regurc("+AMFGP",audiourc)
ril.regurc("+QTTS",audiourc)
--注册以下AT命令的应答处理函数
ril.regrsp("+AUDREC",audiorsp,0)
ril.regrsp("+QTTS",audiorsp,0)

--[[
函数名：setspeakervol
功能  ：设置音频通道的输出音量
参数  ：
		vol：音量等级，取值范围为audio.VOL0到audio.VOL7，audio.VOL0为静音
返回值：无
]]
function setspeakervol(vol)
	audio.setvol(vol)
	speakervol = vol
	--dispatch("SPEAKER_VOLUME_SET_CNF",true)
end

--[[
函数名：getspeakervol
功能  ：读取音频通道的输出音量
参数  ：无
返回值：音量等级
]]
function getspeakervol()
	return speakervol
end

--[[
函数名：setaudiochannel
功能  ：设置音频通道
参数  ：
		channel：音频通道，跟硬件设计有关，用户程序需要根据硬件配置，Air200模块就固定用audiocore.HANDSET
返回值：无
]]
function setaudiochannel(channel)
	audio.setchannel(channel)
	audiochannel = channel
	--dispatch("AUDIO_CHANNEL_SET_CNF",true)
end

--[[
函数名：getaudiochannel
功能  ：读取音频通道
参数  ：无
返回值：音频通道
]]
function getaudiochannel()
	return audiochannel
end

--[[
函数名：setloopback
功能  ：设置回环测试
参数  ：
		flag：是否打开回环测试，true为打开，false为关闭
		typ：测试回环的音频通道，跟硬件设计有关，用户程序需要根据硬件配置
		setvol：是否设置输出的音量，true为设置，false不设置
		vol：输出的音量
返回值：true设置成功，false设置失败
]]
function setloopback(flag,typ,setvol,vol)
	return audio.setloopback(flag,typ,setvol,vol)
end

--[[
函数名：setmicrophonegain
功能  ：设置MIC的音量
参数  ：
		vol：mic音量等级，取值范围为audio.MIC_VOL0到audio.MIC_VOL15，audio.MIC_VOL0为静音
返回值：无
]]
function setmicrophonegain(vol)
	audio.setmicvol(vol)
	microphonevol = vol
	--dispatch("MICROPHONE_GAIN_SET_CNF",true)
end

--[[
函数名：getmicrophonegain
功能  ：读取MIC的音量等级
参数  ：无
返回值：音量等级
]]
function getmicrophonegain()
	return microphonevol
end

--[[
函数名：audiomsg
功能  ：处理底层上报的rtos.MSG_AUDIO外部消息
参数  ：
		msg：play_end_ind，是否正常播放结束
		     play_error_ind，是否播放错误
返回值：无
]]
local function audiomsg(msg)
	if msg.play_end_ind == true then
		if playname then audio.play(playname) return end
		dispatch("AUDIO_PLAY_END_IND")
	elseif msg.play_error_ind == true then
		if playname then playname = nil end
		dispatch("AUDIO_PLAY_ERROR_IND")
	end
end

--注册底层上报的rtos.MSG_AUDIO外部消息的处理函数
sys.regmsg(rtos.MSG_AUDIO,audiomsg)
--默认音频通道设置为RECEIVER，因为Air200模块只支持RECEIVER通道
setaudiochannel(audio.HANDSET)
--默认音量等级设置为4级，4级是中间等级，最低为0级，最高为7级
setspeakervol(audio.VOL4)
--默认MIC音量等级设置为1级，最低为0级，最高为15级
setmicrophonegain(audio.MIC_VOL1)

