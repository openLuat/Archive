
module(...,package.seeall)

function bin2(self,val,data)
	print(self.tag,val,common.binstohexs(data))
end

function new(tag,fmt)
	local o = {tag = tag}

	o.write = bin2

	return o
end
