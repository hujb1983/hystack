local dict = ngx.shared._dict
local rules = {}


--[[
    QPS 是存储访问频率；
]]
function rules.QPS(token, interval)

    local req, _ = dict:get(token)

    if req 
    then
        dict:incr(token, 1)
        req = req + 1
		return req
    end

    dict:set(token, 1, tonumber(interval))
    return 1
end


--[[
    Check QPS..
    -- 计算访问次数
    -- 如果是错误 502 和 504，参数对应到 AccessLocation.lua。
       修改：IPTABLES_SWITCH_ALL_QPS 为 IPTABLES_SWITCH_ERR_QPS
]]
local err
local switchTime = 0
switchTime, err = dict:get("IPTABLES_SWITCH_TIME")

local allQPS = rules.QPS("IPTABLES_SWITCH_ALL_QPS_COUNT", tonumber(switchTime))
local errQPS = rules.QPS("IPTABLES_SWITCH_ERR_QPS_COUNT", tonumber(switchTime))

ngx.var.qps = allQPS
return 0