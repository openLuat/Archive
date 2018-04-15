--[[
模块名称：参数管理模块
模块功能：提供修改，获取参数，保存参数的接口
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)
require"config"

package.path = "/?.lua;".."/?.luae;"..package.path

local configname,econfigname,paraname,para = "/lua/config.lua","/lua/config.luae","/para.lua"

--[[
函数名：restore
功能  ：将参数恢复成默认参数
参数  ：无
返回值：无
]]
function restore()
	local fpara,fconfig = io.open(paraname,"wb"),io.open(configname,"rb")
	if not fconfig then fconfig = io.open(econfigname,"rb") end
	fpara:write(fconfig:read("*a"))
	fpara:close()
	fconfig:close()
	para = config
end

--[[
函数名：serialize
功能  ：将修改后的值写进文件中
参数  ：无
返回值：无
]]
local function serialize(pout,o)
	if type(o) == "number" then
		pout:write(o)
	elseif type(o) == "string" then
		pout:write(string.format("%q", o))
	elseif type(o) == "boolean" then
		pout:write(tostring(o))
	elseif type(o) == "table" then
		pout:write("{\n")
		for k,v in pairs(o) do
			if type(k) == "number" then
				pout:write(" [", k, "] = ")
			elseif type(k) == "string" then
				pout:write(" [\"", k,"\"] = ")
			else
				error("cannot serialize table key " .. type(o))
			end
			serialize(pout,v)
			pout:write(",\n")
		end
		pout:write("}\n")
	else
		error("cannot serialize a " .. type(o))
	end
end

--[[
函数名：upd
功能  ：如果config中的参数没拷贝到para中，将config文件中的参数拷贝到para中，
参数  ：无
返回值：无
]]
local function upd()
	--local f = io.open(paraname,"ab")
	for k,v in pairs(config) do
		if k ~= "_M" and k ~= "_NAME" and k ~= "_PACKAGE" then
			if para[k] == nil then
				--f:write(k, " = ")
				--serialize(f,v)
				--f:write("\n")
				para[k] = v
			end			
		end
	end
	--f:close()
end

--[[
函数名：load
功能  ：加载para文件
参数  ：无
返回值：无
]]
local function load()
	local f = io.open(paraname,"rb")
	if not f or f:read("*a") == "" then
		if f then f:close() end
		restore()
		return
	end
	f:close()
	--加载para文件
	f,para = pcall(require,"para")
    --如果加载para文件失败，恢复成默认参数
	if not f then
		restore()
		return
	end
	upd()
end

--[[
函数名：save
功能  ：存储参数
参数  ：s,swei true才保存，否则不保存
返回值：无
]]
local function save(s)
	if not s then return end
	local f = io.open(paraname,"wb")

	f:write("module(...)\n")

	for k,v in pairs(para) do
		if k ~= "_M" and k ~= "_NAME" and k ~= "_PACKAGE" then
			f:write(k, " = ")
			serialize(f,v)
			f:write("\n")
		end
	end

	f:close()
end

--[[
函数名：set
功能  ：修改参数值
参数  ：k,参数名称
        v 修改后的值
        r 由谁修改的，如果这个参数不为nil，则会分发PARA_CHANGED_IND消息，否则不发送PARA_CHANGED_IND消息
        s 修改后是否马上保存，fasle不保存否则保存
返回值：true
]]
function set(k,v,r,s)
	local bchg
	if type(v) == "table" then
		for kk,vv in pairs(para[k]) do
			if vv ~= v[kk] then bchg = true break end
		end
	else
		bchg = (para[k] ~= v)
	end
	print("nvm.set",bchg,k,v,r,s)
	if bchg then		
		para[k] = v
		save(s or s==nil)
		if r then sys.dispatch("PARA_CHANGED_IND",k,v,r) end
	end
	return true
end

--[[
函数名：sett
功能  ：修改类型为table的参数中的某一元素
参数  ：k,参数名称（参数为table）
        kk，表中元素索引值
        v 修改后的值
        r 由谁修改的，如果这个参数不为nil，则会分发TPARA_CHANGED_IND消息，否则不发送TPARA_CHANGED_IND消息
        s 修改后是否马上保存，fasle不保存否则保存
返回值：true
]]
function sett(k,kk,v,r,s)
	if para[k][kk] ~= v then
		para[k][kk] = v
		save(s or s==nil)
		if r then sys.dispatch("TPARA_CHANGED_IND",k,kk,v,r) end
	end
	return true
end

--[[
函数名：flush
功能  ：保存参数
参数  ：无
返回值：无
]]
function flush()
	save(true)
end

--[[
函数名：get
功能  ：获取参数值
参数  ：k，参数名
返回值：参数值
]]
function get(k)
	if type(para[k]) == "table" then
		local tmp = {}
		for kk,v in pairs(para[k]) do
			tmp[kk] = v
		end
		return tmp
	else
		return para[k]
	end
end

--[[
函数名：gett
功能  ：获取表中某一元素
参数  ：k,参数名称（参数为table）
        kk，表中元素索引值
返回值：表中某一元素
]]
function gett(k,kk)
	return para[k][kk]
end

load()
