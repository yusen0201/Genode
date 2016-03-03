/*
 * \brief  Connection to Loader service
 * \author Christian Prochaska
 * \author Norman Feske
 * \date   2009-10-05
 */

/*
 * Copyright (C) 2009-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
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
