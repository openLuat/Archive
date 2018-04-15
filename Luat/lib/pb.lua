--- SIM电话簿管理
-- @module cc
-- @author 黄洵
-- @license MIT
-- @copyright yunpolice.cn
-- @release 2018.03.10

module(..., package.seeall)
local base = _G
local ril = require"ril"
local sys = require"sys"
local print,tonumber,req = base.print,base.tonumber,ril.request
--local smatch = string.match
local storagecb,readcb,writecb,deletecb

--- 设置电话本存储区域
-- @param str, string类型，存储区域字符串，仅支持"ME"和"SM"
-- @param cb, 设置后的回调函数
-- @return 无
-- @usage pb.setsgorage(str,cb)
function setstorage(str,cb)
	if str=="SM" or str=="ME" then
		storagecb = cb
		req("AT+CPBS=\"" .. str .. "\"" )
	end
end
--- 查找用户名是否存在
-- @param name,string类型
-- @return boole，true存在,false不存在
-- @usage pb.find(name)
function find(name)
	if name == "" or name == nil then
		return	false
	end
	req("AT+CPBF=\"" .. name .. "\"" )
	return true
end

--- 读取一条电话本记录
-- @param index, number类型，电话本在存储区的位置
-- @param cb,读取后的回调函数
-- @usage 无
-- @usage pb.read(1,cb)
function read(index,cb)
	if index == "" or index == nil then
		return false
	end
	readcb = cb
	req("AT+CPBR=" .. index)
end

--- 写一条电话本记录
-- @param index, number类型，电话本在存储区的位置
-- @param name, string类型 ,姓名
-- @param num, number or string类型,号码
-- @param cb, functionl类型，写入后的回调函数
-- @return 无
-- @usage pb.writeitem(1,zhangsan,13233334444,cb)
function writeitem(index,name,num,cb)
	if num == nil or name == nil or index == nil then
		return false
	end
	writecb = cb
	req("AT+CPBW=" .. index .. ",\"" .. num .. "\"," .. "129" .. ",\"" .. name .. "\"" )
	return true
end


--- 删除一条电话本记录
-- @param i, number类型，电话本在存储区的位置
-- @param cb, function类型，删除后的回调函数
-- @return 无
-- @usage pb.deleteitem(1,cb)
function deleteitem(i,cb)
	if i == "" or i == nil then
		return false
	end
	deletecb = cb
	req("AT+CPBW=" .. i)
	return true
end

local function pbrsp(cmd,success,response,intermediate)
	local prefix = string.match(cmd,"AT(%+%u+%?*)")
	intermediate = intermediate or ""

	if prefix == "+CPBF"  then
		local name = string.match(cmd,"AT%+CPBF%s*=%s*\"(%w*)\"")
		if intermediate == "" then
			sys.subscribe("PB_FIND_CNF",success,"","",name)
		else
			for w in string.gmatch(intermediate, "(.-)\r\n") do
				local index,n = string.match(w,"+CPBF:%s*(%d+),\"([#%*%+%d]*)\",%d+,")
				index = index or ""
				n = n or ""
				sys.subscribe("PB_FIND_CNF",success,index,n,name)
			end
		end
	elseif prefix == "+CPBR" then
		local index = string.match(cmd,"AT%+CPBR%s*=%s*(%d+)")
		local num,name = string.match(intermediate,"+CPBR:%s*%d+,\"([#%*%+%d]*)\",%d+,\"(%w*)\"")
		num,name = num or "",name or ""
		sys.subscribe("PB_READ_CNF",success,index,num,name)
		local cb = readcb
		readcb = nil
		if cb then cb(success,name,num) return end
	elseif prefix == "+CPBW" then
		sys.subscribe("PB_WRITE_CNF",success)
		local cb = writecb
		writecb = nil
		if cb then cb(success) return end
		cb = deletecb
		deletecb = nil
		if cb then cb(success) return end
	elseif prefix == "+CPBS?" then
		local storage,used,total = string.match(intermediate,"+CPBS:%s*\"(%u+)\",(%d+),(%d+)")
		used,total = tonumber(used),tonumber(total)
		sys.subscribe("CPBS_READ_CNF",success,storage,used,total)
	elseif prefix == "+CPBS" then
		local cb = storagecb
		storagecb = nil
		if cb then cb(success) return end
    end
end

ril.regrsp("+CPBF",pbrsp)
ril.regrsp("+CPBR",pbrsp)
ril.regrsp("+CPBW",pbrsp)
ril.regrsp("+CPBS",pbrsp)
ril.regrsp("+CPBS?",pbrsp)
