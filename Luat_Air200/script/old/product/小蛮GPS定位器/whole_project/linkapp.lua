--[[
模块名称：linkapp
模块功能：管理socket的接口操作
模块最后修改时间：2017.02.09
]]
module(...,package.seeall)

local lstate,scks,SCK_MAX_CNT = link.getstate,{},4
NORMAL,SVR_CHANGE = 0,1

local function print(...)
	_G.print("linkapp",...)
end

--[[
函数名：checkidx
功能  ：检查idx是否是后效的idx
参数  ：cause,idx,fnm
返回值：无
]]
local function checkidx(cause,idx,fnm)
	if (cause == 0 and idx <= SCK_MAX_CNT) or (cause == 1 and scks[idx]) then
		return true
	else
		print("checkidx "..fnm.." err",idx)
	end
end

--[[
函数名：checkidx1
功能  ：检查idx是否是后效的idx
参数  ：idx,fnm
返回值：true有效，false或nil无效
]]
local function checkidx1(idx,fnm)
	return checkidx(0,idx,fnm) and checkidx(1,idx,fnm)
end

--[[
函数名：getidxbyid
功能  ：由sckid获取idx
参数  ：无
返回值：无
]]
local function getidxbyid(id)
	local i
	for i=1,SCK_MAX_CNT do
		if scks[i] and scks[i].id == id then return i end
	end
end

--[[
函数名：conrstpara
功能  ：重置连接相关参数
参数  ：idx,suc
返回值：无
]]
local function conrstpara(idx,suc)
	if not checkidx(1,idx,"conrstpara") then return end
	scks[idx].conretry,scks[idx].concause = 0
	if not suc then	scks[idx].sndpending = {} end
	scks[idx].sndingitem,scks[idx].waitingrspitem = {},{}
end

--[[
函数名：sndrstpara
功能  ：重置发送参数
参数  ：无
返回值：无
]]
local function sndrstpara(idx)
	if not checkidx(1,idx,"sndrstpara") then return end
	scks[idx].sndretry,scks[idx].sndingitem = 0,{}
end

--[[
函数名：discrstpara
功能  ：重置断开原因值
参数  ：无
返回值：无
]]
local function discrstpara(idx)
	if not checkidx(1,idx,"discrstpara") then return end
	scks[idx].discause = nil
end

--[[
函数名：rsumscksnd
功能  ：恢复sck的发送
参数  ：无
返回值：无
]]
local function rsumscksnd(idx)
	if not checkidx(1,idx,"rsumscksnd") then return end
	if lstate(scks[idx].id) ~= "CONNECTED" then
		return link.connect(scks[idx].id,scks[idx].prot,scks[idx].addr,scks[idx].port)
	else
		if #scks[idx].sndpending ~= 0 and not scks[idx].sndingitem.data and not scks[idx].waitingrspitem.data then
			local item = table.remove(scks[idx].sndpending,1)
			if link.send(scks[idx].id,item.data) then
				scks[idx].sndingitem = item
			else
				table.insert(scks[idx].sndpending,1,item)
			end
		end
	end
	return true
end

--[[
函数名：setwaitingrspitem
功能  ：设置等待应答项
参数  ：idx,item
返回值：无
]]
function setwaitingrspitem(idx,item)
	if not checkidx(1,idx,"setwaitingrspitem") then return end
	scks[idx].waitingrspitem = item
	if not item.data then
		rsumscksnd(idx)
	end
end

--[[
函数名：conack
功能  ：连接ack
参数  ：idx,cause,suc
返回值：无
]]
local function conack(idx,cause,suc)
	conrstpara(idx,suc)
    --如果等待发送的队列不为空且连接失败，则依次回复发送失败事件，否则回复CONNECT事件
	if #scks[idx].sndpending ~= 0 and not suc then
		while #scks[idx].sndpending ~= 0 do
			scks[idx].rsp(idx,"SEND",suc,table.remove(scks[idx].sndpending,1))
		end
	else
		scks[idx].rsp(idx,"CONNECT",suc,cause)
	end
end

--[[
函数名：sndnxt
功能  ：发送下一个数据
参数  ：id,idx
返回值：无
]]
local function sndnxt(id,idx)
	local item = table.remove(scks[idx].sndpending,1)
	if link.send(id,item.data) then
		scks[idx].sndingitem = item
	else
		table.insert(scks[idx].sndpending,1,item)
	end
end

--[[
函数名：sndack
功能  ：发送数据ack
参数  ：idx,suc,item
返回值：无
]]
local function sndack(idx,suc,item)
	sndrstpara(idx)
	scks[idx].rsp(idx,"SEND",suc,item)
end

--[[
函数名：sckrsp
功能  ：sck应答事件处理函数，对连接，断开发送状态等事件作出应答
参数  ：无
返回值：无
]]
local function sckrsp(id,evt,val)
	local idx = getidxbyid(id)
	if not idx then print("sckrsp err idx",id,evt,val) return end
	print("sckrsp",id,evt,val)

	if evt == "CONNECT" then
		local cause = scks[idx].concause
		if val ~= "CONNECT OK" then
			scks[idx].conretry = scks[idx].conretry + 1
			if scks[idx].conretry >= 1 then
				conack(idx,cause,false)
			else
				if not link.connect(id,scks[idx].prot,scks[idx].addr,scks[idx].port) then
					conack(idx,cause,false)
				end
			end
		else
			conack(idx,cause,true)
			if #scks[idx].sndpending ~= 0 and not scks[idx].sndingitem.data then
				sndnxt(id,idx)
			end
		end
	elseif evt == "SEND" then
		local item = scks[idx].sndingitem
		if val ~= "SEND OK" then
			scks[idx].sndretry = scks[idx].sndretry + 1
			if scks[idx].sndretry >= 1 then
				sndack(idx,false,item)
			else
				if not link.send(id,item.data) then
					sndack(idx,false,item)
				end
			end
		else
			sndack(idx,true,item)
			if #scks[idx].sndpending ~= 0 and not scks[idx].sndingitem.data and not scks[idx].waitingrspitem.data then
				sndnxt(id,idx)
			end
		end
	elseif evt == "DISCONNECT" then
		local cause = scks[idx].discause
		discrstpara(idx)
		if cause == SVR_CHANGE or #scks[idx].sndpending ~= 0 then
			link.connect(id,scks[idx].prot,scks[idx].addr,scks[idx].port)
			scks[idx].concause = cause
		end
		scks[idx].rsp(idx,"DISCONNECT",true,cause)
	elseif evt == "STATE" and val == "CLOSED" then
		if #scks[idx].sndpending ~= 0 then
			link.connect(id,scks[idx].prot,scks[idx].addr,scks[idx].port)
		end
		scks[idx].rsp(idx,evt,val,nil)
	else
		scks[idx].rsp(idx,evt,val,nil)
	end
end

--[[
函数名：sckrcv
功能  ：sck接收数据
参数  ：id,data
返回值：无
]]
local function sckrcv(id,data)
	scks[getidxbyid(id)].rcv(getidxbyid(id),data)
end

--[[
函数名：sckclrsnding
功能  ：清空等待发送的队列
参数  ：idx
返回值：无
]]
function sckclrsnding(idx)
	if not checkidx1(idx,"sckclrsnding") then return end
	scks[idx].sndpending = {}
end

--[[
函数名：sckinit
功能  ：sck初始化，讲sck信息加入scks表中
参数  ：idx,id,cause,prot,addr,port,rsp,rcv,discause
返回值：无
]]
local function sckinit(idx,id,cause,prot,addr,port,rsp,rcv,discause)
	scks[idx] =
	{
		id = id,
		addr = addr,
		port = port,
		prot = prot,
		conretry = 0,
		sndretry = 0,
		sndpending = {},
		sndingitem = {},
		waitingrspitem = {},
		rsp = rsp,
		rcv = rcv,
		concause = cause,
		discause = discause,
	}
end

--[[
函数名：sckcreate
功能  ：创建一个sck
参数  ：idx,cause,prot,addr,port,rsp,rcv
返回值：true创建成功，false或nil失败
]]
function sckcreate(idx,cause,prot,addr,port,rsp,rcv)
	if not checkidx(0,idx,"sckcreate") or checkidx(1,idx,"sckcreate") then return end
	sckinit(idx,link.open(sckrsp,sckrcv),cause,prot,addr,port,rsp,rcv)
	return true
end

--[[
函数名：sckconn
功能  ：socket连接操作
参数  ：无
返回值：true连接成功，false或nil失败
]]
function sckconn(idx,cause,prot,addr,port,rsp,rcv)
	if not checkidx(0,idx,"sckconn") then return end
	local discause,sckid
	if scks[idx] then
		sckid = scks[idx].id
		if link.getstate(sckid) == "CONNECTED" then
			if scks[idx].addr == addr and scks[idx].port == port and scks[idx].prot == prot then
				return true
			else
				if link.disconnect(sckid) then discause = cause	end
			end
		else
			if not link.connect(sckid,prot,addr,port) then
				print("sckconn fail1")
				return false
			end
		end
	else
		sckid = link.open(sckrsp,sckrcv)
		if not link.connect(sckid,prot,addr,port) then
			print("sckconn fail2")
			return false
		end
	end
	sckinit(idx,sckid,cause,prot,addr,port,rsp,rcv,discause)

	return true
end

--[[
函数名：scksnd
功能  ：发送数据
参数  ：无
返回值：true
]]
function scksnd(idx,data,para,pos,ins)
	if not checkidx1(idx,"scksnd") then return end
	if not data or string.len(data) == 0 then print("scksnd data empty") return end

	local sckid = scks[idx].id
	local item,tail = {data = data, para = para},#scks[idx].sndpending+1
    
    --如果未连接成功，先进行连接操作，连接成功后将带发送的数据插入待发送队列
	if lstate(sckid) ~= "CONNECTED" then
		local res = link.connect(sckid,scks[idx].prot,scks[idx].addr,scks[idx].port)
		if res or (not res and ins) then
			table.insert(scks[idx].sndpending,pos or tail,item)
		end
    --连接成功
	else
        --如果有正在处理的item，则插入等待队列，否则直接发送
		if scks[idx].sndingitem.data or scks[idx].waitingrspitem.data then
			table.insert(scks[idx].sndpending,pos or tail,item)
		else
			if link.send(sckid,data) then
				scks[idx].sndingitem = item
			end
		end
	end
	return true
end

--[[
函数名：sckdisc
功能  ：断开sck
参数  ：idx
返回值：true断开成功否则断开失败
]]
function sckdisc(idx)
	if not checkidx1(idx,"sckdisc") then return end
	return link.disconnect(scks[idx].id)
end

--[[
函数名：issckactive
功能  ：判断sck是否为连接状态
参数  ：idx
返回值：true为连接状态否则不是
]]
function issckactive(idx)
	if not checkidx1(idx,"issckactive") then return end
	return link.getstate(scks[idx].id) == "CONNECTED"
end

link.setconnectnoretrestart(true,90000)
