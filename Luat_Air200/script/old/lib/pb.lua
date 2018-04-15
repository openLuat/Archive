local base = _G
local ril = require"ril"
local sys = require"sys"
local string = require"string"

module("pb")

local print = base.print
local req = ril.request
local dispatch = sys.dispatch
local smatch = string.match
local tonumber = base.tonumber

function find(name)
	if name == "" or name == nil then
		return	false
	end
	req("AT+CPBF=\"" .. name .. "\"" )
	return true
end
--read a pb entry
function read(index)
	if index == "" or index == nil then
		return false
	end
	req("AT+CPBR=" .. index)
end

function writeitem(index,name,num)
	if num == nil or name == nil or index == nil then
		return false
	end
	req("AT+CPBW=" .. index .. ",\"" .. num .. "\"," .. "129" .. ",\"" .. name .. "\"" )
	return true
end

function deleteitem(i)
	if i == "" or i == nil then
		return false
	end
	req("AT+CPBW=" .. i)
	return true
end

local function pbrsp(cmd,success,response,intermediate)
	if success == false then
		return
	end
	local prefix = smatch(cmd,"AT(%+%u+%?*)")
	intermediate = intermediate or ""

	if prefix == "+CPBF"  then
		local name = string.match(cmd,"AT%+CPBF%s*=%s*\"(%w*)\"")
		if intermediate == "" then
			dispatch("PB_FIND_CNF",success,"","",name)
		else
			for w in string.gmatch(intermediate, "(.-)\r\n") do
				local index,n = smatch(w,"+CPBF:%s*(%d+),\"([#%*%+%d]*)\",%d+,")
				index = index or ""
				n = n or ""
				dispatch("PB_FIND_CNF",success,index,n,name)
			end
		end
	elseif prefix == "+CPBR" then
		local index = string.match(cmd,"AT%+CPBR%s*=%s*(%d+)")
		if index == nil then
			return
		end
		if intermediate == "" then
			dispatch("PB_READ_CNF",success,index,"","")
		else
			local n,name = smatch(intermediate,"+CPBR:%s*%d+,\"([#%*%+%d]*)\",%d+,\"(%w*)\"")
			n = n or ""
			name = name or ""
			dispatch("PB_READ_CNF",success,index,n,name)
		end
	elseif prefix == "+CPBS?" then
		local storage,used,total = smatch(intermediate,"+CPBS:%s*\"(%u+)\",(%d+),(%d+)")
		used,total = tonumber(used),tonumber(total)
		dispatch("CPBS_READ_CNF",success,storage,used,total)
    end
end

ril.regrsp("+CPBF",pbrsp)
ril.regrsp("+CPBR",pbrsp)
ril.regrsp("+CPBS?",pbrsp)
