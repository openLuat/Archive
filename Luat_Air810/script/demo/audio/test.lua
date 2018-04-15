module(...,package.seeall)

require"audio"
require"common"

--音频播放优先级，对应audio.play接口中的priority参数；数值越大，优先级越高，用户根据自己的需求设置优先级
--PWRON：开机铃声
--CALL：来电铃声
--SMS：新短信铃声
PWRON,CALL,SMS = 2,1,0

local function testcb(r)
	print("testcb",r)
end

--播放音频文件测试接口，每次打开一行代码进行测试
local function testplayfile()
	--单次播放来电铃声，默认音量等级
	--audio.play(CALL,"FILE","/ldata/call.mp3")
	--单次播放来电铃声，音量等级7
	--audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7)
	--单次播放来电铃声，音量等级7，播放结束或者出错调用testcb回调函数
	--audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7,testcb)
	--循环播放来电铃声，音量等级7，没有循环间隔(一次播放结束后，立即播放下一次)
	audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7,nil,true)
	--循环播放来电铃声，音量等级7，循环间隔为2000毫秒
	--audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7,nil,true,2000)
end

--播放冲突测试接口，每次打开一个if语句进行测试
local function testplayconflict()	

	if true then
		--循环播放来电铃声
		audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7,nil,true)
		--5秒钟后，循环播放开机铃声
		sys.timer_start(audio.play,5000,PWRON,"FILE","/ldata/pwron.mp3",audiocore.VOL7,nil,true)		
	end

	
	--[[
	if true then
		--循环播放来电铃声
		audio.play(CALL,"FILE","/ldata/call.mp3",audiocore.VOL7,nil,true)
		--5秒钟后，尝试循环播放新短信铃声，但是优先级不够，不会播放
		sys.timer_start(audio.play,5000,SMS,"FILE","/ldata/sms.mp3",audiocore.VOL7,nil,true)
		
	end
	]]	
end


--每次打开下面的一行代码进行测试
sys.timer_start(testplayfile,5000)
--sys.timer_start(testplayconflict,5000)
