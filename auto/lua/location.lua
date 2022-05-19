local redis = require("resty.redis");

local paramUri = ngx.var.uri;
local paramHost = ngx.var.host;
local paramKey = paramHost..paramUri

if adCache == "" or adCache == nil then

	local red = redis:new()
	red:set_timeout(2000)

	local ok, err = red:connect("127.0.0.1", 6379)
	local value = red:get(paramKey)
	if (value ~= ngx.nil)
	then
		ngx.say(value)
	end
else
