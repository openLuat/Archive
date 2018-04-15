module(...,package.seeall)

local i2cid,intregaddr = 1,0x1A

local function print(...)
  _G.print("gsensor",...)
end

local function clrint()
	print("clrint 1")
	if pins.get(pinscfg.GSENSOR) then
		print("clrint 2")
		i2c.read(i2cid,intregaddr,1)
	end
end

local function init()
	local i2cslaveaddr = 0x0E
	
	pmd.ldoset(3, pmd.LDO_VMC)

	if i2c.setup(i2cid,i2c.SLOW,i2cslaveaddr) ~= i2c.SLOW then
		print("init fail")
		return
	end
	
	local req_id=i2c.read(i2cid,0xf,1)
	print("init id: ", string.format("%02X", string.byte(req_id)))
	
	local cmd,i = {0x1B,0x00,   0x6A,0x01,  0x1E,0x20, 0x21,0x04,  0x1B,0x00,  0x1B,0x9A,  0x1B,0x9A}
	for i=1,#cmd,2 do
		i2c.write(i2cid,cmd[i],cmd[i+1])
		print("init",string.format("%02X",cmd[i]),string.format("%02X",string.byte(i2c.read(i2cid,cmd[i],1))))
	end
	clrint()
end

local function ind(v)
  print("ind",v)
  if v == true then
    clrint()
    print("ind GSENSOR_SHK_IND")
    sys.dispatch("GSENSOR_SHK_IND")
  end
end

init()
pinscfg.GSENSOR.intcb=ind
sys.timer_loop_start(clrint,30000)
