/*
 * \brief  Client-side interface of the Hello service
 * \author Björn Döbel
 * \date   2008-03-20
 */

/*
 * Copyright (C) 2008-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__HELLO_SESSION_H__CLIENT_H_
#define _INCLUDE__HELLO_SESSION_H__CLIENT_H_

#include <hello_session/hello_session.h>
#include <base/rpc_client.h>
#include <base/printf.h>

#include <trace/timestamp.h>
namespace Hello {
	using namespace Genode;
	struct Session_client : Genode::Rpc_client<Hello::Session>
	{
		Session_client(Genode::Capability<Hello::Session> cap)
		: Genode::Rpc_client<Hello::Session>(cap) { }
			
		Trace::Timestamp a;

		void say_hello()
		{
			a = Trace::timestamp();
			printf("red client start say hello %u \n", a);
			PDBG("Saying Hello from red_client");
			call<Rpc_say_hello>();
		}

		int add(int a, int b)
		{
			return call<Rpc_add>(a, b);
		}
	};
}

#endif /* _INCLUDE__HELLO_SESSION_H__CLIENT_H_ */
