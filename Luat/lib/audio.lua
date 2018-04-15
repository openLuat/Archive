--- 模块功能：音频播放
-- @module audio
-- @author 稀饭放姜
-- @license MIT
-- @copyright openLuat
-- @release 2017.10.21
require "common"
require "misc"
module(..., package.seeall)
--- 播放音频文件
-- @string fn ,包含路径的文件名,如果是flash内置路径为"/ldata/"
-- @return boole ,true 播放成功，false，播放失败
-- @usage local let = audio.play("/ldata/call.mp3")
function play(fn)
    return audiocore.play(fn)
end
--- 停止播放当前音频文件
-- @return 无
-- @usage audio.stop()
function stop()
    audiocore.stop()
end
--- 播放TTS文件
-- @string s，UTF-8格式的字符串，支持中英文
-- @return 无
-- @usage audio.play_utf8_tts("欢迎光临")
function play_utf8_tts(s)
    ril.request("AT+QTTS=1")
    local str = string.tohex(common.utf8toucs2(s))
    ril.request(string.format('AT+QTTS=2,"%s"', str))
end
--- 播放TTS文件
-- @string s，GB2312格式的字符串，支持中英文
-- @return 无
-- @usage audio.play_gb2312_tts("欢迎光临")
function play_gb2312_tts(s)
    ril.request("AT+QTTS=1")
    local str = string.tohex(common.gb2312toucs2(s))
    ril.request(string.format('AT+QTTS=2,"%s"', str))
end
--- 停止播放TTS文件
-- @return 无
-- @usage audio.stop_tts()
function stop_tts()
    ril.request("AT+QTTS=3")
    ril.request("AT+QTTS=0")
end
--- 设置播放通道
-- @number chan,播放输出通道：audiocore.HANDSET 、 audiocore.EARPIECE 、 audiocore.LOUDSPEAKER 、audiocore.BLUETOOTH 、 audiocore.FM 、 audiocore.FM_LP 、 audiocore.TV 、audiocore.AUX_HANSET 、 audiocore.AUX_LOUDSPEAKER 、audiocore.AUX_EARPIECE 、 audiocore.DUMMY_HANDSET 、audiocore.DUMMY_AUX_HANDSET 、 audiocore.DUMMY_LOUDSPEAKER 、audiocore.DUMMY_AUX_LOUDSPEAKER
-- @return boole , 设置成功返回true,失败返回false
function setChannel(chan)
    return audiocore.setchannel(chan)
end
--- 设置音量
-- @number vol,音量值为0-7
-- @return boole , 设置成功返回true,失败返回false
-- @usage audio.setVolume(7)
function setVolume(vol)
    return audiocore.setvol(vol)
end
--- 设置麦克音量
-- @number vol,音量值为0-15
-- @return boole , 设置成功返回true,失败返回false
-- @usage audio.setMicVolume(14)
function setMicVolume(vol)
    return audiocore.setmicvol(vol)
end
--- TTS播报当前时间
function chime()
    local t = misc.getClock()
    local s = "现在时刻--北京时间： " .. t.hour .. "点  " .. t.min .. "分  " .. t.sec .. "秒  ,,,,今天是阳历--：" .. t.year .. "年  " .. t.month .. "月  " .. t.day .. "号,  星期" .. misc.getWeek()
    play_utf8_tts(s)
end

setVolume(5)