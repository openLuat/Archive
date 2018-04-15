--[[
模块名称：录音模块
模块功能：提供实时录音接口，将录音内容上报服务器接口
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

--ing true：开始录音操作 false或者nil未有录音操作或者录音结束
--rcdlen录音时长
--rcdsta “RCDING”正在录音 "RCDRPT" 正在上报录音文件
--rcdtyp 0实时录音（录完上报后台） 1：本地拾音，录完不上报后台
local ing,rcdlen,rcdsta,rcdtyp
--RCD_ID 录音文件编号
--RCD_FILE录音文件名
--RCD_SPLITSIZE未用到
local RCD_ID,RCD_FILE,RCD_SPLITSIZE = 1,"/RecDir/rec001",1002
--seq报文序列号
--unitlen录音文件分包每包最长长度
--way 发送方式，0：录完再发 1：边录边发
--total报文总条数
--cur当前报文索引
local seq,unitlen,way,total,cur=0,1024,0
--存放录音命令的缓冲区，当某个时间段连续收到多个录音命令，则依次录音并发送录音文件
local buf={}

local function print(...)
	_G.print("rcd",...)
end

--[[
函数名：start
功能  ：开始录音，录音前先删除之前的录音文件
参数  ：无
返回值：无
]]
local function start()
	print("start",ing,rcdlen)
    --标记有拾音请求正在进行
	ing = true
    --删除以前的录音文件
	os.remove(RCD_FILE)
    --开始录音
	audio.beginrecord(RCD_ID,rcdlen*1000)
end

--[[local function stoprcd()
	print("stoprcd")
	audio.endrecord(RCD_ID)
	rcdendind(true)
end]]

--[[
函数名：rcdcnf
功能  ：AUDIO_RECORD_CNF消息处理函数
参数  ：suc，suc为true表示开始录音否则录音失败
返回值：true
]]
local function rcdcnf(suc)
	print("rcdcnf",suc,rcdsta)
	if suc and not rcdsta then
		rcdsta = "RCDING"
	end
	return true
end

--[[
函数名：getrcddata
功能  ：获取录音文件指定索引的数据
参数  ：s报文序列号
        idx报文索引
返回值：录音文件指定索引的数据
]]
function getrcddata(s,idx)
	local f,rt = io.open(RCD_FILE,"rb")
    --如果打开文件失败，返回内容为空“”
	if not f then print("getrcddata can not open file",f) return "" end
	if not f:seek("set",(idx-1)*unitlen) then print("getfdata seek err") return "" end
    --读取一个单元长度的数据
	rt = f:read(unitlen)
	f:close()
	print("getrcddata",string.len(rt),s,idx)
	return rt or ""
end

--[[
函数名：getrcdinf
功能  ：获取报文序列号，当前录音文件的报文总条数
参数  ：无
返回值：报文序列号，当前录音文件的报文总条数，第一条报文索引值
]]
local function getrcdinf()
	local f,rt = io.open(RCD_FILE,"rb")
	if not f then print("getrcdinf can not open file",f) return nil,0,0 end
	local size = f:seek("end")
	if not size or size == 0 then print("getrcdinf seek err") return nil,0,0 end
	f:close()
    --分配序列号（0-255）
	seq = (seq+1>255) and 0 or (seq+1)
    --计算报文总条数
	total,cur = (size-1)/unitlen+1,1
	print("getrcdinf",size,seq,total,cur)
	return seq,(size-1)/unitlen+1,1
end

--[[
函数名：rcdendind
功能  ：录音结束处理函数
参数  ：suc：true，录音成功；false录音失败
返回值：true
]]
local function rcdendind(suc)
	print("rcdendind",suc,rcdsta)
	--sys.timer_stop(stoprcd)
    --录音成功
	if suc and rcdsta=="RCDING" then
		rcdsta="RCDRPT"  
		collectgarbage()
        --获取录音文件信息
		getrcdinf()
		--开始发送第一包录音文件
        sys.dispatch("SND_QRYRCD_REQ",seq,way,total,cur,rcdlen,getrcddata(seq,cur))	
		print("rcdendind",suc,rcdsta,seq,total,cur,rcdlen)
	else
        --录音失败，删除录音文件
		os.remove(RCD_FILE)
		ing,rcdlen,rcdsta = nil
	end
	return true
end

--[[
函数名：rcdind
功能  ：录音请求处理函数
参数  ：length：录音长度
        typ：录音类型，0实时录音（录完上报后台） 1：本地拾音，录完不上报后台
返回值：true
]]
local function rcdind(length,typ)
	print("rcdind",length,ing)
    --记录拾音类型，暂只支持实时录音
	rcdtyp = typ
	if typ ~= 0 then return print("rcdind can not support local record ") end
	if length <= 0 then print("rcdind length can not be 0") return end
    --如果此时有拾音请求正在进行中，将本请求存入缓冲区，等前面的拾音请求完成后再执行此拾音请求
	if ing then
		table.insert(buf,{length,typ})
	else
        --如果没有拾音请求正在执行，马上执行本次拾音请求
		--if length and (length > 5000 or length < 0) then length = 5000 end
		rcdlen = (length or 5000)/1000
		start()
	end
	return true
end

--[[
函数名：sndcnf
功能  ：录音请求处理函数
参数  ：res ，true发送成功，false发送失败
        s 报文序列号
        c 报文当前索引
返回值：true
]]
local function sndcnf(res,s,c)
	print("sndcnf",res,s,c,seq,cur,total)  
    --拾音报文发送成功
	if res and tonumber(s)==seq and tonumber(c)== cur then
		cur = cur+1
		print("sndcnf111",res,s,c,seq,cur,total)  
        --如果录音文件没发送完，则继续下一包录音文件的发送
		if cur<=total then
			print("sndcnf222",res,s,c,seq,cur,total)   
			sys.dispatch("SND_QRYRCD_REQ",seq,way,total,cur,rcdlen,getrcddata(seq,cur))  
			return true
		end 
	end
    --所有录音数据发送完毕，删除录音文件
	os.remove(RCD_FILE)
	cur,total,ing,rcdlen,rcdsta = nil
    --如果buf中还有没执行完的拾音请求，继续执行拾音请求操作
	if #buf>0 then
		local rcdinfo=table.remove(buf,1)
		rcdind(rcdinfo[1],rcdinfo[2])
	end
    return true
end

--注册app处理函数
local procer = {
	QRY_RCD_IND = rcdind,
	AUDIO_RECORD_CNF = rcdcnf,
	AUDIO_RECORD_IND = rcdendind,
	SND_QRYRCD_CNF=sndcnf,
}
sys.regapp(procer)
