/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

 /*
  *	API版本.代码版本.发布类型.测试次数
  *  1.3.2.0
  *  API	版本: 第(1)套
  *  代码版本: 更新版本
  *  发布类型: develop(1) | master(2) | beta(3) | stable(>3)
  */

#define hystack_version				010301
#define MODULE_VERSION				"1.3.5"
#define MODULE_VER					"hystack-" MODULE_VERSION

#define HYSTACK						"hystack"
#define HYSTACK_VAR					"hystack-cdn"
#define MODULE_OLDPID_EXT			".oldbin"

#if (WINX)
#define MODULE_PREFIX				"D:\\hystack\\auto"
#define MODULE_PREFIX_CONF			"D:\\hystack\\auto\\conf"
#define MODULE_ERROR_LOG			"D:\\hystack\\auto\\logs"
#define MODULE_PID_PATH				"D:\\hystack\\auto\\logs"
#else
#define MODULE_PREFIX				"/usr/local/asnode/"
#define MODULE_PREFIX_CONF			"/usr/local/asnode/conf"
#define MODULE_SOCKET_DATA			"/usr/local/asnode/data"
#define MODULE_ERROR_LOG			"/usr/local/asnode/logs"
#define MODULE_PID_PATH				"/usr/local/asnode/sbin"
#endif


#define MODULE_UNSET_INDEX		(unsigned int) -1

#define MODULE_V1										\
		MODULE_UNSET_INDEX, MODULE_UNSET_INDEX,			\
		nullptr, 0, 0, hystack_version, nullptr

#define MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0

