#ifndef HV_ICMP_H_
#define HV_ICMP_H_

// @param cnt: ping count
// @return: ok count
// @note: printd $CC -DPRINT_DEBUG
int ping(const char* host, int cnt);

#endif // HV_ICMP_H_
