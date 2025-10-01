/*
 * @Author: AZhang
 * @Date: 2025-10-01 23:48:28
 * @LastEditTime: 2025-10-01 23:48:36
 * @FilePath: /t5_ai_coffee/src/common/hv_net.h
 * @Description:  
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
 */
#ifndef __LIB_NET_H__
#define __LIB_NET_H__

int HV_NET_GetMacAddr(const char* pIface, char* pMac1, char* pMac2);
int HV_NET_SetMacAddr(const char* pIface, char* pMac);

int HV_NET_GetIpAddr(const char* pIface, char* pIP);
int HV_NET_SetIpAddr(const char* pIface, char* pIP);

int HV_NET_GetNetmask(const char* pIface, char* pNetmask);
int HV_NET_SetNetmask(const char* pIface, char* pNetmask);

int HV_NET_GetGateway(const char* pIface, char* pcGateway);
int HV_NET_SetGateway(const char* pIface, char* pcGateway);

int HV_NET_GetDNS(char* pcDNS);
int HV_NET_SetDNS(char* pcDNS);
int HV_NET_GetDNS_ex(char* pcDNS1, char* pcDNS2);
int HV_NET_SetDNS_ex(char* pcDomainDNS, char* pcWifiDNS);

int HV_NET_GetBrocastAddr(const char* pIface, char* pAddr);

int HV_NET_GetPptpAddr(const char* pIface, char* pAddr);

int HV_NET_DetectRoutingAddr(const char* pIface, char* pRouting);

int HV_NET_GetLinkStatus(const char* pIface);

int HV_NET_CheckNetStatus(void);

int HV_NET_CheckNicStatus(const char* nic_name);

/// @brief ״̬
/// @param nic_name 
/// @return 
int HV_NET_CheckNicStatusUp(const char* nic_name);


#endif
