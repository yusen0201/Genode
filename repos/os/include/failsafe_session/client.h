/*
 * \brief  Client-side failsafe-session interface
 * \author Yusen Wang
 * \date   2016-04-07
 */


#ifndef _INCLUDE__FAILSAFE_SESSION__CLIENT_H_
#define _INCLUDE__FAILSAFE_SESSION__CLIENT_H_

#include <failsafe_session/failsafe_session.h>
#include <base/rpc_client.h>
#include <os/alarm.h>

namespace Failsafe { struct Session_client; }


struct Failsafe::Session_client : Genode::Rpc_client<Session>
{
	explicit Session_client(Capability<Session> session)
	: Rpc_client<Session>(session) { }

	
	
	void fault_sigh(Signal_context_capability sigh) override {
		call<Rpc_fault_sigh>(sigh); }

};

#endif /* _INCLUDE__PLUGIN_SESSION__CLIENT_H_ */
