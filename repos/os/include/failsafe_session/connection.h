/*
 * \brief  Connection to failsafe service
 * \author Yusen Wang 
 * \date   2016-04-07
 */



#ifndef _INCLUDE__FAILSAFE_SESSION__CONNECTION_H_
#define _INCLUDE__FAILSAFE_SESSION__CONNECTION_H_

#include <failsafe_session/client.h>
#include <base/connection.h>

namespace Failsafe { struct Connection; }


struct Failsafe::Connection : Genode::Connection<Session>, Session_client
{
	Connection(size_t ram_quota)
	:
		Genode::Connection<Session>(session("ram_quota=%zd", ram_quota)),
		Session_client(cap())
	{ }
};

#endif /* _INCLUDE__FAILSAFE_SESSION__CONNECTION_H_ */
