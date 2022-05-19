--[[
	curl --unix-socket /var/run/nginx/nginx.sock http://localhost/nginx-defense?param='eyJpcCI6IjExMy4yNDYuMjI5LjQ1IiwiZG9tYWluIjoiaHVhbmFuMi5nb2Rsb2dpYy50b3AifQ=='\&opt='cc_allow_pass_temporary'
]]

local rjson = require "cjson.safe"

local dict = ngx.shared._dict

local opt = ngx.var.arg_opt
local param = ngx.var.arg_param
local verifier = ngx.decode_base64(param);

local ccJson = rjson.decode(verifier)
if (ccJson ~= nil)
then
	local ccToken = "REDIRECT_"..ccJson.domain.."_"..ccJson.ip;
	if (opt == "cc_allow_pass_temporary") then
		dict:set(ccToken, "true", 30)
	end

	if (opt == "cc_add_to_blacklist") then
		dict:set(ccToken, "false", 30)
	end
end
