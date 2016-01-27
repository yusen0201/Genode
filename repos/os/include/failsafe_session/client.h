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

#ifndef _INCLUDE__LOADER_SESSION__CLIENT_H_
#define _INCLUDE__LOADER_SESSION__CLIENT_H_

#include <failsafe_session/loader_session.h>
#include <base/rpc_client.h>
#include <os/alarm.h>

namespace Loader { struct Session_client; }


struct Loader::Session_client : Genode::Rpc_client<Session>
{
	explicit Session_client(Capability<Session> session)
	: Rpc_client<Session>(session) { }

	Dataspace_capability alloc_rom_module(Name const &name,
	                                      size_t size) override {
		return call<Rpc_alloc_rom_module>(name, size); }

	void commit_rom_module(Name const &name) override {
		call<Rpc_commit_rom_module>(name); }

	void ram_quota(size_t quantum) override {
		call<Rpc_ram_quota>(quantum); }

	
	void fault_sigh(Signal_context_capability sigh) override {
		call<Rpc_fault_sigh>(sigh); }

	void start(Name const &binary, Name const &label = "",
	           Native_pd_args const &pd_args = Native_pd_args()) override {
		call<Rpc_start>(binary, label, pd_args); }

};

#endif /* _INCLUDE__PLUGIN_SESSION__CLIENT_H_ */
