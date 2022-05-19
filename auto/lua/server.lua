
local rjson = require "cjson.safe"
local dict = ngx.shared._dict
local spider_list = ngx.shared._spiderlist
local white_list = ngx.shared._whitelist
local black_list = ngx.shared._blacklist

--[[
    local ngx = {}
    ngx.var = {}
    ngx.var.remote_addr = "113.246.229.206";
    ngx.var.host = "www.baidu.com";
]]

-- cc black to list;
-- ngx.redirect(redirect_url, 302);
-- ngx.exit(403);
local defanse_key = "REDIRECT_"..ngx.var.host.."_"..ngx.var.remote_addr;
local defanse_url, err = dict:get(defanse_key)
if (defanse_url ~= nil) then
    if (defanse_url ~= "true") then
        ngx.exit(403)
    end
    return 0
end

--[[
    string.split
    string.to_char_array
]]
function string.split(str, delimiter)
    if str==nil or str=='' or delimiter==nil then
        return nil
    end
    
    local result = {}
    for match in (str..delimiter):gmatch("(.-)"..delimiter) 
    do
        table.insert(result, match)
    end
    return result
end

function string.to_char_array(str)
    str = str or ""
    local array = {}
    local len = string.len(str)
    while str do
        local font_utf = string.byte(str,1)
        if font_utf == nil then
            break
        end
        if font_utf > 127 then 
            local tmp = string.sub(str,1,3)
            table.insert(array,tmp)
            str = string.sub(str,4,len)
        else
            local tmp = string.sub(str,1,1)
            table.insert(array,tmp)
            str = string.sub(str,2,len)
        end
    end
    return array
end

--[[
    ips.to_doc
    ips.compare
]]
local match_ip = {};
function match_ip.to_doc(ip_string)
    local res = {}
    for k in string.gmatch(ip_string, "%d+") do
        table.insert(res, k)
    end
    return res
end

function match_ip.compare(val, src)
    local list_ = string.split(val,"/")
    if #list_ ~= 2 then
        if val == src then
            return true
        end
        return false
    end
    
    local size = 0
    local code = tonumber(list_[2]);
    if code == 8 then 
        size = 3
    elseif code == 16 then 
        size = 2
    elseif code == 24 then 
        size = 1
    end
    
    local doc_val = match_ip.to_doc(val_)
    local doc_src = match_ip.to_doc(src)

    for  i, v in pairs(doc_val) do
        if size == 0 then
            break
        end
        if v ~=  doc_src[i] then
            return false
        end
        size = size - 1
    end
    return true
end

--[[
    match_ac.creat_tree
    match_ac.push_queue
    match_ac.pop_queue
    match_ac.create_branch_node
    match_ac.build_node_failed
    match_ac.query
    match_ac.replace
    match_ac.get_mask_worlds
]]
local match_ac = {}
match_ac.world_list = {}
match_ac.queue = {}

match_ac.trie = {
    char = "root",
    index = 1,
    is_end = false,
    node_dic = {},
    faild= nil,
}

function match_ac.creat_tree(str_words)
    local word = tostring(str_words)
    match_ac.create_branch_node(match_ac.trie,string.to_char_array(word))
    match_ac.build_node_failed(match_ac.trie)
end

function match_ac.push_queue(node)
    table.insert( match_ac.queue, node)
end

function match_ac.pop_queue()
    local tb = match_ac.queue[1]
    table.remove(match_ac.queue,1)
    return tb
end

function match_ac.create_branch_node(root,char_array)
    local index = 1;
    local cur_node = root
    local address_index = 1
    while index <= #char_array 
    do
        local char = char_array[index]
        if cur_node.node_dic[char] ~= nil then
            cur_node =  cur_node.node_dic[char]
            
			if  not cur_node.is_end then
                cur_node.is_end = index == #char_array
            end
        else
            local _node = {}
            _node.char = char
            _node.index = index
			if not _node.is_end then
                _node.is_end = index == #char_array
            end
            _node.node_dic = {}
            _node.faild = {}
            cur_node.node_dic[char] = _node
            cur_node = cur_node.node_dic[char]
        end
        index = index + 1 
    end
end

function match_ac.build_node_failed(root)
    root.faild = nil
    match_ac.push_queue(root)
    local parent  = {}
    local tmp_faild = {}
    while #match_ac.queue > 0 
	do
        parent = match_ac.pop_queue()
        for char,node in pairs(parent.node_dic) 
		do
            if parent.char == "root" then
                node.faild = parent
            else
                tmp_faild = parent.faild
                while tmp_faild 
				do
                    if tmp_faild.node_dic[char] then
                        node.faild = tmp_faild.node_dic[char]
                        break;
                    end
                    tmp_faild = tmp_faild.fail
                    if not tmp_faild then
                        node.faild = root
                    end 
                end
            end
            match_ac.push_queue(node)
        end
    end
end

function match_ac.query(arry)
    local  p = match_ac.trie
    local  len = #arry
    local index_list = {}
    local temp_list = {}
    local ar = arry:to_char_array()
	for i=1,len 
	do
        local x = ar[i]
        while (not p.node_dic[x]) and p.faild  
        do
            p = p.faild
            if not p.faild then
                temp_list = {}
            end
        end
        p = p.node_dic[x]
        if not p then
            p = match_ac.trie
        else
            table.insert( temp_list, i)
        end
        if p.is_end then
            for i, index in ipairs(temp_list) 
			do
                table.insert(index_list, index)
            end
            temp_list = {}
        end
    end
    return index_list
end

function match_ac.replace(str,mark)
    mark = mark or "*"
    local arry = str:to_char_array()
    local mask_list = match_ac.query(arry)
    local _str = ""
    for i,index in ipairs(mask_list) 
    do
        arry[index] = mark
    end
    for i,char in ipairs(arry) 
    do
        _str = _str..char
    end
    return _str
end

function match_ac.get_mask_worlds()
    return match_ac.world_list
end

--[[
    match.equal
    match.contain
    match.AC
]]
local match_result = {};
match_result.unknow = 0;
match_result.succeed = 1;
match_result.failed = 2;

function string.compare_ac(des, src)
	match_ac.creat_tree(src)
	local result = match_ac.query(des)
	if (#result == #src) then
        return true
    end
    return false
end

local match = {}
function match.equal(tag, opt, val)
	
	local src
	if (tag == nil) or (val == nil) or (opt == nil) then
		return match_result.unknow;
	end
	
	if (tag == "ip") then
		src = match_ip.compare(val, ngx.var.remote_addr)
	elseif (tag == "host") then
		src = string.find(ngx.var.host, val);
	elseif (tag == "uri") then
		src = string.find(ngx.var.uri, val);
	elseif (tag == "req_uri") then 
		src = string.find(ngx.var.request_uri, val);
	elseif (tag == "req_mothed") then
		src = string.find(ngx.var.request_method, val);
	elseif (tag == "accept_language") then 
		src = string.compare_ac(ngx.var.http_accept_language, val);
	elseif (tag == "user_agent") then
		src = string.compare_ac(ngx.var.http_user_agent, val);
	elseif (tag == "referer") then 
		local referer_ = ngx.var.scheme.."://"..ngx.var.host..ngx.var.request_uri
		src = string.find(referer_, val);
	elseif (tag == "country_iso_code") then
		src = string.find(ngx.var.country_iso_code, val);
	end

	if (src == nil) then
        src = false
    end

    if (src ~= false) then
        if (opt == "=") then
                return match_result.succeed
        end
    else
        if (opt == "!=") then
                return match_result.succeed
        end
    end
    return match_result.failed
end

function match.contain(tag, opt, val)

	local src = false
	local match_count = 0;

	if (tag == nil) or (val == nil) or (opt == nil) then
		return match_result.unknow;
	end
	
    local list = string.split(val, ",")
	for idx, word in pairs(list)
	do
		if (tag == "ip") then
			src = match_ip.compare(word, ngx.var.remote_addr)
		elseif (tag == "host") then
			src = string.find(word, ngx.var.host);
		elseif (tag == "uri") then
			src = string.find(word, ngx.var.uri);
		elseif (tag == "req_uri") then 
			src = string.find(word, ngx.var.request_uri);
		elseif (tag == "req_mothed") then
			src = string.find(word, ngx.var.request_method);
		elseif (tag == "accept_language") then 
			src = string.compare_ac(ngx.var.http_accept_language, val);
		elseif (tag == "user_agent") then 
			src = string.compare_ac(word, ngx.var.http_user_agent);
		elseif (tag == "referer") then 
			local referer_ = ngx.var.scheme.."://"..ngx.var.host..ngx.var.request_uri
			src = string.find(referer_, val);
		elseif (tag == "country_iso_code") then
			src = string.find(word, ngx.var.country_iso_code);
		end
		
		if (src == nil) then
			src = false
		end
		
		if (src ~= false) then
			match_count = match_count + 1
		end
	end

	if (opt == "contain") and (match_count > 0) then
		return match_result.succeed
	end

	if (opt == "!contain") and (match_count == 0) then
		return match_result.succeed
	end

	return match_result.failed
end

function match.AC(tag, opt, val)
	
	local src = false
	local match_count = 0;
	
	if (tag == nil) or (val == nil) or (opt == nil) then
		return match_result.unknow;
	end
	
    local list = string.split(val, ",")
	for idx, word in pairs(list)
	do
		if (tag == "ip") then
			src = match_ip.compare(word, ngx.var.remote_addr)
		elseif (tag == "host") then
			src = string.find(ngx.var.host, word);
		elseif (tag == "uri") then
			src = string.find(ngx.var.uri, word);
		elseif (tag == "req_uri") then 
			src = string.find(ngx.var.request_uri, word);
		elseif (tag == "req_mothed") then
			src = string.find(ngx.var.request_method, word);
		elseif (tag == "accept_language") then 
			src = string.compare_ac(ngx.var.http_accept_language, word);
		elseif (tag == "user_agent") then 
			src = string.compare_ac(ngx.var.http_user_agent, word);
		elseif (tag == "referer") then 
			local referer_ = ngx.var.scheme.."://"..ngx.var.host..ngx.var.request_uri
			src = string.find(referer_, val);
		elseif (tag == "country_iso_code") then
			src = string.find(ngx.var.country_iso_code, word);
		end
		
		if (src == nil) then
			src = false
		end
		
		if (src ~= false) then
			match_count = match_count + 1
		end
	end

	if (opt == "AC") and (match_count > 0) then
		return match_result.succeed
	end

	if (opt == "!AC") and (match_count == 0) then
		return match_result.succeed
	end
	
	return match_result.failed
end

--[[
    rules.iptables
	rules.match_item
	rules.match_acl
	rules.match_cc
	rules.request_count
	rules.filter_item
	rules.check_cc_level
	rules.check_cc
]]
local rules={}
local rules_result = {};
rules_result.unknow = 0;
rules_result.succeed = 1;
rules_result.failed = 2;
rules_result.closed = 3;
rules_result.whitelist = 4;
rules_result.blacklist= 5;
rules_result.passed= 6;

function rules.iptables(addr, vhost)
    local addr_key = vhost..addr
    local flag, err = dict:get("IPTABLES")
    if flag == 0 then
        return rules_result.closed
    end
    local val = spider_list:get(addr_key)
    if val ~= nil then
        return rules_result.whitelist
    end
    val = white_list:get(addr_key)
    if val ~= nil then
        return rules_result.whitelist
    end
    val = black_list:get(addr_key)
    if val ~= nil or val == true then
        return rules_result.blacklist
    end
    return rules_result.passed
end

-- Check Iptables..
local ret_rules = rules.iptables(ngx.var.remote_addr, ngx.var.host)
if (ret_rules == rules_result.closed) or (ret_rules == rules_result.whitelist) then
    return 0
elseif (ret_rules == rules_result.blacklist) then
    ngx.exit(403)
end

function rules.match_QPS(token, interval)
    local req, _ = dict:get(token)
    if req then
        dict:incr(token, 1)
        req = req + 1
		return req
    end
    dict:set(token, 1, tonumber(interval))
    return 1
end


-- Check rules.switch..
local rule_switch = {}
rule_switch.open = 0
rule_switch.err_qps = 10
rule_switch.all_qps = 500
rule_switch.id = 2
rule_switch.time = 300

function rules.switch(qps)
    local err = 0
    rule_switch.open, err = dict:get("IPTABLES_SWITCH")
    rule_switch.err_qps, err = dict:get("IPTABLES_SWITCH_ERR_QPS")
    rule_switch.all_qps, err = dict:get("IPTABLES_SWITCH_ALL_QPS")
    rule_switch.id, err = dict:get("IPTABLES_SWITCH_ID")
    rule_switch.time, err = dict:get("IPTABLES_SWITCH_TIME")

	if (rule_switch.open == 0) then 
		return 0
	end
    
	if (tonumber(qps) > tonumber(rule_switch.all_qps)) then
		if (rule_switch.time == ngx.null) or (rule_switch.time == nil) then 
			return -1
		end 
		local start, err = dict:get("IPTABLES_SWITCH_START")
		if (start == nil or start == ngx.null) then
			dict:set("IPTABLES_SWITCH_START", 1,  tonumber(rule_switch.time))
		end
		return -1
	end
	return 0
end

-- Check QPS.. Error 502 | 504
local err, cc_order
cc_order, err = dict:get("IPTABLES_RULE")

local switch_time
switch_time, err = dict:get("IPTABLES_SWITCH_TIME")

local qps
qps = rules.match_QPS("IPTABLES_SWITCH_ALL_QPS_COUNT", switch_time)
if (rules.switch(qps) == -1) then
	cc_order = rule_switch.id;
end

qps = rules.match_QPS("IPTABLES_SWITCH_ERR_QPS_COUNT", switch_time)
if (rules.switch(qps) == -1) then
	cc_order = rule_switch.id;
end

function rules.match_item(str_match)
    local ret

    if (str_match == nil) then
        return match_result.succeed;
    end

	if (str_match == "") then
		return match_result.succeed;
	end
	
    local list_ = string.split(str_match, "&")
    for m, mv in pairs(list_) 
    do
        local ary = {}
        for w in string.gmatch(mv, "%S+") do
			table.insert(ary, w)
        end
		
		if (#ary ~= 3) then
			goto continue
		end
		
		if (ary[2] == "=" or ary[2] == "!=") then
			ret = match.equal(ary[1], ary[2], ary[3]);
			if ( ret == match_result.succeed or ret == match_result.failed) then
				return ret
			end
		end

		if (ary[2] == "contain" or ary[2] == "!contain") then
			ret = match.contain(ary[1], ary[2], ary[3]);
			if ( ret == match_result.succeed or ret == match_result.failed) then
				return ret
			end
		end

		if (ary[2] == "AC" or ary[2] == "!AC") then
			ret = match.AC(ary[1], ary[2], ary[3]);
			if ( ret == match_result.succeed or ret == match_result.failed) then
				return ret
			end
		end
		::continue::
    end
    return match_result.failed
end


function rules.match_acl(acl_id)
    local ret
    local acl_token = "ACL_"..acl_id.."_RULES"
	local json_value, err = dict:get(acl_token)
    if (json_value == nil or json_value == ngx.null) then
        return match_result.failed
    end
	
	local opt;
    local val = rjson.decode(json_value)
    if (val ~= nil or val ~= ngx.null) then
        for k, v in pairs(val)
        do
			opt = v.opt
            if (v ~= nil) then
                ret = rules.match_item(v.match)
                if (ret == match_result.succeed or ret == match_result.failed) then
					break
                end
            end
        end
    end

	if (ret == match_result.succeed) then
		if (opt == "allow") then
			return match_result.succeed;
		end
	end
    return match_result.failed;
end

-- Check ACL..
local match_acl
local match_key = "DOMAIN_"..ngx.var.domainId.."_ACL";
match_acl, err = dict:get(match_key)
if (match_acl ~= nil) then
	local ret = rules.match_acl(tonumber(match_acl))
	if (ret ~= match_result.no_rule) then
		if (ret == -1) then
			ngx.exit(403)
		end
	end
end

function rules.request_count(token, interval, max_time)
    local req, _ = dict:incr(token, 1, 0, tonumber(interval))
    if req > max_time then
		return rules_result.failed
    end

    return rules_result.passed
end

function rules.filter_item(ary, cc_order, vhost, addr, filter)
	
	local verify = true
	local tag = ary[1]
	local inter_value = ary[2]
	local max_count = tonumber(ary[3])
    
    if (inter_value == nil or max_count == nil) then 
        return rules_result.succeed
    end 
    
    local path = nil 
	path = tag.." "..inter_value.." "..max_count

	if (#ary == 4) then
		path = path.." "..ary[4]
	end

    local token = nil
	if (tag == "req_rate") then
		verify = false
        token = "CC_"..cc_order.."_REQRATE_"..vhost.."_R_"..addr.."_V_"..filter
	end

	--[[
	elseif (tag == "captcha_filter") then
        token = "CC_"..cc_order.."_RULES_"..addr.."_VERIFIERCODE_"..vhost
	elseif (tag == "delay_jump_filter") then 
        token = "CC_"..cc_order.."_RULES_"..addr.."_JUMPFILTER_"..vhost
	elseif (tag == "slide_filter") then
        token = "CC_"..cc_order.."_RULES_"..addr.."_SLIDEFILTER_"..vhost
	elseif (tag == "click_filter") then 
        token = "CC_"..cc_order.."_RULES_"..addr.."_CLICKFILTER_"..vhost
    elseif (tag == "browser_verify_auto") then
        token = "CC_"..cc_order.."_RULES_"..addr.."_FIVESECOND_"..vhost
    end
	]]
    
    if (token == nil) then
        return rules_result.succeed, ""
    end
	
	local ret = rules.request_count(token, inter_value, max_count)
	if (ret == rules_result.passed) then
		if (verify == true) then
            return rules_result.failed, path
		end
	end
	
	return ret, path
end

function rules.check_cc_level(qps, cc_order, addr, vhost, domain)

    local level, err, idx, maxIdx, token

    idx = 1
    maxIdx = 10

    while (idx < maxIdx)
    do
        token = "DOMAIN_"..domain.."_CC_ID_"..idx
        local new_id, err= dict:get(token)
        if (new_id == nil) then
			return cc_order
        end

        token = "DOMAIN_"..domain.."_CC_LEVEL_"..idx
        level, err= dict:get(token)
        if (qps < level) then
			return cc_order
        end
		
        cc_order = new_id
        idx = idx + 1
    end

    return cc_order
end

local rules_filter_item
function rules.check_cc(cc_order, addr, vhost, domain)

    local ret, ret_filter, err_html
    local token = "CC_"..cc_order.."_RULES"

	local json_value, _ = dict:get(token)
    if (json_value == nil) or (json_value == ngx.null) then
        return rules_result.passed
    end
    
    local val = rjson.decode(json_value)
    if (val == nil) or (val == ngx.null) then
        return rules_result.passed
    end

	local filter_check
	for k, v in pairs(val)
	do
        if (v == nil) then
            return 0
        end

        ret = rules.match_item(v.match)
        if (ret == rules_result.succeed) then
		
            if (v.filter==nil) then
                return 0
            end
            
            local filter_list = string.split(v.filter, ";")
			if ( #filter_list ~= 2) then
				return 0
			end

			local mv = filter_list[1]
			filter_check = filter_list[2]

			local ary = {}
			for w in string.gmatch(mv, "%S+") do
				table.insert(ary, w)
			end

			rules_filter_item = ary;
			ret_filter, err_html = rules.filter_item(ary, cc_order, addr, vhost, v.match)
			if (ret_filter == rules_result.failed) then
				break
			end
        end
	end
	
	return ret_filter, filter_check
end

-- Check CC
local cc_token = "DOMAIN_"..ngx.var.domainId.."_CC";
local cc_check, err = dict:get(cc_token)
if (cc_check ~= nil) 
then
	cc_order = rules.check_cc_level(qps, cc_order, ngx.var.remote_addr, ngx.var.host, ngx.var.domainId)
	if ( cc_order == nil ) then
		return 0
	end

	local filter_path
	ret_rules, filter_path = rules.check_cc(cc_order, ngx.var.remote_addr, ngx.var.host, ngx.var.domainId)
	if (ret_rules == rules_result.failed) then
		if (filter_path ~= nil) 
        then
			local tag = rules_filter_item[1]
			local inter_value = rules_filter_item[2]
			local max_count = tonumber(rules_filter_item[3])
			
			local param_url = "ip="..ngx.var.remote_addr.."&&"
			param_url = param_url.."cc="..cc_order.."&&"
			param_url = param_url.."nodeid="..ngx.var.nodeId.."&&"
			param_url = param_url.."filter="..filter_path.."&&"
			param_url = param_url.."url="..ngx.var.scheme.."://"..ngx.var.server_name..ngx.var.request_uri
			param_url = param_url.."&&domain="..ngx.var.server_name
			
			local verify64 = "Nodeparam="..ngx.encode_base64(param_url)
			local cc_token = "REDIRECT_"..ngx.var.host.."_"..ngx.var.remote_addr;

			dict:set(cc_token, verify64, tonumber(inter_value))
			ngx.exec("/verify_safe", verify64);
		end
	end
end
