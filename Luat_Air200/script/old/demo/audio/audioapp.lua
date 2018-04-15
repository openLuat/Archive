--[[
模块名称：音频管理
模块功能：音频播放、停止、冲突管理
模块最后修改时间：2017.02.23
]]

module(...,package.seeall)

--音频类型，数值越小，优先级越高，用户根据自己的需求设置优先级
--PWRON：开机铃声
--CALL：来电铃声
--SMS：新短信铃声
--TTS：TTS播放
PWRON,CALL,SMS,TTS = 0,1,2,3

--styp：当前播放的音频类型
--spath：当前播放的音频文件路径
--svol：当前播放音量
--scb：当前播放结束或者出错的回调函数
--sdup：当前播放的音频是否需要重复播放
--sduprd：如果sdup为true，此值表示重复播放的间隔(单位毫秒)，默认无间隔
--spending：将要播放的音频是否需要正在播放的音频异步结束后，再播放
local styp,spath,svol,scb,sdup,sduprd

--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上audioapp前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("audioapp",...)
end

--[[
函数名：playbegin
功能  ：关闭上次播放后，再播放本次请求
参数  ：
		typ：音频类型，参考PWRON,CALL,SMS
		path：音频文件路径
		vol：播放音量，取值范围audiocore.VOL0到audiocore.VOL7。此参数可选
		cb：音频播放结束或者出错时的回调函数，回调时包含一个参数：0表示播放成功结束；1表示播放出错；2表示播放优先级不够，没有播放。此参数可选
		dup：是否循环播放，true循环，false或者nil不循环。此参数可选
		duprd：播放间隔(单位毫秒)，dup为true时，此值才有意义。此参数可选
返回值：调用成功返回true，否则返回nil
]]
local function playbegin(typ,path,vol,cb,dup,duprd)
	print("playbegin")
	--重新赋值当前播放参数
	styp,spath,svol,scb,sdup,sduprd,spending = typ,path,vol,cb,dup,duprd

	--如果存在音量参数，设置音量
	if vol then
		audio.setspeakervol(vol)
    end
	
	--调用播放接口成功
	if (typ==TTS and audio.playtts(path)) or (typ~=TTS and audio.play(path,dup and (not duprd or duprd==0))) then
		return true
	--调用播放接口失败
	else
		styp,spath,svol,scb,sdup,sduprd,spending = nil
	end
end

--[[
函数名：play
功能  ：播放音频
参数  ：
		typ：音频类型，参考PWRON,CALL,SMS
		path：音频文件路径
		vol：播放音量，取值范围audiocore.VOL0到audiocore.VOL7。此参数可选
		cb：音频播放结束或者出错时的回调函数，回调时包含一个参数：0表示播放成功结束；1表示播放出错；2表示播放优先级不够，没有播放。此参数可选
		dup：是否循环播放，true循环，false或者nil不循环。此参数可选
		duprd：播放间隔(单位毫秒)，dup为true时，此值才有意义。此参数可选
返回值：调用成功返回true，否则返回nil
]]
function play(typ,path,vol,cb,dup,duprd)
	print("play",typ,path,vol,cb,dup,duprd,styp)
	--有音频正在播放
	if styp then
		--正在播放的音频优先级 低于 将要播放的音频优先级
		if typ < styp then
			--如果正在播放的音频有回调函数，则执行回调，传入参数2
			if scb then scb(2) end
			--停止正在播放的音频
			if not stop() then
				styp,spath,svol,scb,sdup,sduprd,spending = typ,path,vol,cb,dup,duprd,true
				return
			end
		--正在播放的音频优先级 高于 将要播放的音频优先级
		elseif typ > styp then
			--直接返回nil，不允许播放
			return
		--正在播放的音频优先级 等于 将要播放的音频优先级，有两种情况(1、正在循环播放；2、用户重复调用接口播放同一音频类型)
		else
			--如果是第2种情况，直接返回；第1中情况，直接往下走
			if not sdup then
				return
			end
		end
	end

	playbegin(typ,path,vol,cb,dup,duprd)
end

--[[
函数名：stop
功能  ：停止音频播放
参数  ：无
返回值：如果可以成功同步停止，返回true，否则返回nil
]]
function stop()
	local typ = styp
	styp,spath,svol,scb,sdup,sduprd,spending = nil
	--停止循环播放定时器
	sys.timer_stop_all(play)
	--停止音频播放
	audio.stop()
	if typ==TTS then audio.stoptts() return end
	return true
end

--[[
函数名：playend
功能  ：音频播放成功结束处理函数
参数  ：无
返回值：无
]]
local function playend()
	print("playend",sdup,sduprd)
	if styp==TTS and not sdup then audio.stoptts() end
	--需要重复播放
	if sdup then
		--存在重复播放间隔
		if sduprd then
			sys.timer_start(play,sduprd,styp,spath,svol,scb,sdup,sduprd)
		--不存在重复播放间隔
		elseif styp==TTS then
			play(styp,spath,svol,scb,sdup,sduprd)
		end
	--不需要重复播放
	else
		--如果正在播放的音频有回调函数，则执行回调，传入参数0
		if scb then scb(0) end
		styp,spath,svol,scb,sdup,sduprd,spending = nil
	end
end

--[[
函数名：playerr
功能  ：音频播放失败处理函数
参数  ：无
返回值：无
]]
local function playerr()
	print("playerr")
	if styp==TTS then audio.stoptts() end
	--如果正在播放的音频有回调函数，则执行回调，传入参数1
	if scb then scb(1) end
	styp,spath,svol,scb,sdup,sduprd,spending = nil
end

--[[
函数名：ttstopind
功能  ：调用audio.stoptts()接口后，tts停止播放后的消息处理函数
参数  ：无
返回值：无
]]
local function ttstopind()
	print("ttstopind",spending)
	if spending then playbegin(styp,spath,svol,scb,sdup,sduprd) end
end

--音频播放消息处理函数表
local procer =
{
	AUDIO_PLAY_END_IND = playend,
	AUDIO_PLAY_ERROR_IND = playerr,
	TTS_STOP_IND = ttstopind,
}
--注册音频播放消息处理函数
sys.regapp(procer)
