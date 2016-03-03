/*
 * \brief  Client-side loader-session interface
 * \author Christian Prochaska
 * \date   2009-10-05
 */

/*
 * Copyright (C) 2009-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
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
