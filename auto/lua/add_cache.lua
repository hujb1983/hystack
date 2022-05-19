local redis = require "resty.redis";

local red = redis:new();
red:set_timeout(1000);

local ok, err = red:connect("127.0.0.1", 6379);
if not ok then
	ngx.say("failed to connect redis ", err);
	return;
end

local paramAddr = ngx.var.uri;
local isCache = red:get(paramAddr);
if (isCache == nil) then
	ngx.say("is not cache.");
else
	ngx.say("is cache.");
end

local ok, err = red:close();
return 0;
