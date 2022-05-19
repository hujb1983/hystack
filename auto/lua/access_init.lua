--[[
	
]]

local dict = ngx.shared._dict
local spider_list = ngx.shared._spiderlist
local white_list = ngx.shared._whitelist
local black_list = ngx.shared._blacklist

local path = "/usr/local/openresty/nginx/conf/lua/"
local rjson = require "cjson.safe"

function string.split(str, delimiter)
    if (str == nil) or (str == '') or (delimiter == nil) then
        return nil
    end
    
    local result = {}
    for match in (str..delimiter):gmatch("(.-)"..delimiter) 
    do
        table.insert(result, match)
    end
    return result
end


-- spider_ips
local spider_path = path.."spider_ips.json"
local spider_file = io.open(spider_path, "r")
if (spider_file ~= nil) then

    local spider_str = nil
    if (spider_file ~= nil) then
        spider_str = spider_file:read("*a")
        spider_file:close()
    end
    
    local token
    local spider_json = rjson.decode(spider_str)
    if (spider_json ~= nil) then

        for _, list in pairs(spider_json)
        do
            for _, ip in pairs(list)
            do
                token = ip
                spider_list:set(token, true)
            end
        end
    end
end


--  domain
local domain_str = nil
local domain_path = path.."domain.json"
local domain_file = io.open(domain_path, "r")
if (domain_file ~= nil) then

    if (domain_file ~= nil) then
        domain_str = domain_file:read("*a")
        domain_file:close()
    end

    local token
    local domain_json = rjson.decode(domain_str)
    if (domain_json ~= nil) then

        local acl_id
        for _, value in pairs(domain_json)
        do
            token = "DOMAIN_"..value.domain.."_ACL";
            if (value.acl_status == "on") then
                acl_id = tonumber(value.acl_id)
                dict:set(token, acl_id, 0)
            end

            token = "DOMAIN_"..value.domain.."_CC";
            if (value.cc_switch == "on") then
                dict:set(token, true, 0)
            end
            
            for idx, val_cc in pairs(value.cc)
            do
                if (val_cc.status == "on") then
                    token = "DOMAIN_"..value.domain.."_CC_LEVEL_"..idx
                    dict:set(token, val_cc.level, 0)

					token = "DOMAIN_"..value.domain.."_CC_ID_"..idx
                    dict:set(token, val_cc.id, 0)
                end
            end
        end

    end
end

--  iptables 指向全局配置参数的json文件
local ips_str = nil
local ips_path = path.."iptables.json"
local ips_file = io.open(ips_path, "r")
if (ips_file ~= nil) then

    if (ips_file ~= nil) then
        ips_str = ips_file:read("*a")
        ips_file:close()
    end
    
    local token
    local ips_json = rjson.decode(ips_str)
    if (ips_json ~= nil) then

        if (ips_json.iptables == 0) then
            dict:set("IPTABLES", 0, 0)
        end
        
        if (ips_json.iptables == 1) then

            dict:set("IPTABLES", 1, 0)

            token = "IPTABLES_RULE";
            dict:set(token, ips_json.rules, 0)

            local autoSwitch = string.split( ips_json.autoSwitch, " ")
            if (autoSwitch ~= nil and #autoSwitch == 5) then
                local switch = autoSwitch[1]
                dict:set("IPTABLES_SWITCH", switch, 0)
                local errorQPS = autoSwitch[2]
                dict:set("IPTABLES_SWITCH_ERR_QPS", errorQPS, 0)
                local allQPS = autoSwitch[3]
                dict:set("IPTABLES_SWITCH_ALL_QPS", allQPS, 0)
                local swithRule = autoSwitch[4]
                dict:set("IPTABLES_SWITCH_ID", swithRule, 0)
                local switchTime = autoSwitch[5]
                dict:set("IPTABLES_SWITCH_TIME", switchTime, 0)
            end

            local blackTime = ips_json.blackTime
            local blackAddr = string.split( ips_json.black_list, ";")
            if (blackAddr ~= nil) then
                for _, blackIP in pairs(blackAddr)
                do
                    token = blackIP
                    black_list:set(token, true, blackTime)
                end
            end
            
            local whiteTime = tonumber(ips_json.whiteTime)
            local whiteAddr = string.split( ips_json.white_list, ";")
            if (whiteAddr ~= nil) then
                for _, whiteIP in pairs(whiteAddr)
                do
                    token = whiteIP
                    white_list:set(token, true, whiteTime)
                end
            end
        end
        
    end
end


--  acl
local acl_path = path.."acl.json"
local acl_file = io.open(acl_path, "r")
if (acl_file ~= nil) then

    local acl_str = nil
    if (acl_file ~= nil) then
        acl_str = acl_file:read("*a")
        acl_file:close()
    end

    local token
    local aclJson = rjson.decode(acl_str)
    if (aclJson ~= nil) then
        for _, domain in pairs(aclJson)
        do
            token = "ACL_"..domain.id.."_RULES"
            dict:set(token, rjson.encode(domain.rules), 0)
        end
    end
end


-- cc
local cc_path = path.."cc.json"
local cc_file = io.open(cc_path, "r")
if (cc_file ~= nil) then

    local cc_str = nil
    if (cc_file ~= nil) then
        cc_str = cc_file:read("*a")
        cc_file:close()
    end

    local token
    local ccJson = rjson.decode(cc_str)
    if (ccJson ~= nil) then
        for k, val in pairs(ccJson)
        do
            token = "CC_"..val.id.."_RULES"
            dict:set(token, rjson.encode(val.rules), 0)
        end
    end
end


-- ccwhite
local cc_white_path = path.."cc_white.json"
local cc_white_file = io.open(cc_white_path, "r")
if (cc_white_file ~= nil) then

    local cc_white_str = nil
    if (cc_white_file ~= nil) then
		cc_white_str = cc_white_file:read("*a")
		cc_white_file:close()
    end

    local token
    local cc_white_json =  rjson.decode(cc_white_str)
    if (cc_white_json ~= nil) then
        for host, list in pairs(cc_white_json) 
        do
            for j, ip in pairs(list) 
            do
                token = host..tostring(ip)
                white_list:set(token, true)
            end
        end
    end
end


-- ccblack
local cc_black_path = path.."cc_black.json"
local cc_black_file = io.open(cc_black_path, "r")
if (cc_black_file ~= nil) then

    local cc_black_str = nil
    if (cc_black_file ~= nil) then
        cc_black_str = cc_black_file:read("*a")
        cc_black_file:close()
    end

    local cc_black_json = rjson.decode(cc_black_str)
    if (cc_black_json ~= nil) then
        for host, list in pairs(cc_black_json) 
        do
            for j, ip in pairs(list) 
            do
				local token = host..tostring(ip)
                black_list:set(token, true)
            end
        end
    end
end
