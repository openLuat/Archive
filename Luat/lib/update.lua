--- 模块功能：远程升级
-- @module ntp
-- @author 朱天华
-- @license MIT
-- @copyright openLuat
-- @release 2017.11.14
require "misc"
require "utils"
require "socket"
require "log"
local sbyte, ssub = string.byte, string.sub
module(..., package.seeall)
-- 重试次数
local RETRY = 3
-- 升级包保存路径
local UPD_PATH = "/luazip/update.bin"
---  自动连接服务器，检查升级，下载升级包s
-- @return 无
-- @usage update.run()
function run()
	sys.taskInit(
		function()
			local cnt,finished = 1
			for cnt=1,RETRY do
				while not socket.isReady() do sys.waitUntil('IP_READY_IND') end
				local sck = socket.tcp()			
				if sck:connect("iot.openluat.com","80") then
					local url = "/api/site/firmware_upgrade?project_key=".._G.PRODUCT_KEY
						.."&imei="..misc.getimei().."&device_key="..misc.getsn()
						.."&firmware_name=".._G.PROJECT.."_"..rtos.get_version().."&version=".._G.VERSION
					if sck:send("GET "..url.." HTTP/1.1\r\nConnection: keep-alive\r\nHost: iot.openluat.com\r\n\r\n") then
						os.remove(UPD_PATH)
						local rcvBuf,statusCode,contentLen,getBody = ""
						while true do
							local result,data = sck:recv(10000)
							if not result then log.error("update.recv timeout") break end
							if getBody then
							--if true then
								if not io.writefile(UPD_PATH,data,"a+") then log.error("update.writefile error") break end
								local fileSize = io.filesize(UPD_PATH)
								log.info("update.filesize",fileSize)
								finished = (io.filesize(UPD_PATH)==contentLen)
								--finished = (io.filesize(UPD_PATH)==47151)
								if finished then log.info("update.download success") break end
							else
								rcvBuf = rcvBuf..data
								local _,d = string.find(rcvBuf,"\r\n\r\n")
								if d then
									statusCode = string.match(rcvBuf,"HTTP/1.1 (%d+)")
									if statusCode~="200" then log.error("update.statusCode error",statusCode) break end
									
									contentLen = string.match(rcvBuf,"Content%-Length: (%d+)")
									if not contentLen or contentLen=="0" then log.error("update.contentLen error",contentLen) break end
									contentLen = tonumber(contentLen)
									
									getBody = true
									if not io.writefile(UPD_PATH,string.sub(rcvBuf,d+1,-1)) then log.error("update.writefile error") break end
									rcvBuf = ""
								end								
							end
						end
						
					end
				end
				sck:close()
				if finished then
					break
				else
					os.remove(UPD_PATH)
				end
				sys.wait(5000)
			end
			
			if finished then sys.publish('FOTA_DOWNLOAD_FINISH') end
		end
	)
end
