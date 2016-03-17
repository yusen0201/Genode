/*
 * \brief  Process-creation interface
 * \author Norman Feske
 * \date   2006-06-22
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__PROCESS_H_
#define _INCLUDE__BASE__PROCESS_H_

#include <ram_session/capability.h>
#include <rm_session/client.h>
#include <pd_session/client.h>
#include <cpu_session/client.h>
#include <parent/capability.h>

namespace Genode { class Process; }


class Genode::Process
{
	private:

		Dataspace_capability elf;
		Pd_session_client  _pd_session_client;
		Thread_capability  _thread0_cap;
		Cpu_session_client _cpu_session_client;
		Rm_session_client  _rm_session_client;

		static Dataspace_capability _dynamic_linker_cap;		

		addr_t _entry;
		const char *_name;
	public:

		/**
		 * Constructor
		 *
		 * \param elf_data_ds   dataspace that contains the elf binary
		 * \param pd_session    the new protection domain
		 * \param ram_session   RAM session providing the BSS for the
		 *                      new protection domain
		 * \param cpu_session   CPU session for the new protection domain
		 * \param rm_session    RM session for the new protection domain
		 * \param parent        parent of the new protection domain
		 * \param name          name of protection domain (can be used
         *                      for debugging)
		 *
		 * The dataspace 'elf_data_ds' can be read-only.
		 *
		 * On construction of a protection domain, execution of the initial
		 * thread is started immediately.
		 */
		Process(Dataspace_capability    elf_data_ds,
		        Pd_session_capability   pd_session,
		        Ram_session_capability  ram_session,
		        Cpu_session_capability  cpu_session,
		        Rm_session_capability   rm_session,
		        Parent_capability       parent,
		        char const             *name);

		/**
		 * Destructor
		 *
		 * When called, the protection domain gets killed.
		 */
		~Process();

		static void dynamic_linker(Dataspace_capability dynamic_linker_cap)
		{
			_dynamic_linker_cap = dynamic_linker_cap;
		}

		Thread_capability main_thread_cap() const { return _thread0_cap; }
		
		void fast_restart()
		{
			//Pager_capability pager;
			//	pager = _rm_session_client.add_client(_thread0_cap);
			//_cpu_session_client.set_pager(_thread0_cap, pager);
			//_cpu_session_client.start(_thread0_cap, _entry, 0);	
			//_cpu_session_client.resume(_thread0_cap);	

			//if (!pd_session_cap.valid())
			//return;
			try { 
				PDBG("0");
				//_cpu_session_client.kill_thread(_thread0_cap);
				PDBG("old thread killed"); }
			catch (Genode::Ipc_error) {PDBG("kill thread failed"); }

			enum Local_exception
			{
				THREAD_FAIL, ELF_FAIL, ASSIGN_PARENT_FAIL, THREAD_ADD_FAIL,
				THREAD_BIND_FAIL, THREAD_PAGER_FAIL, THREAD_START_FAIL,
			};

			PDBG("1");
			/* XXX this only catches local exceptions */

			/* FIXME find sane quota values or make them configurable */
			try {
				int err;

			PDBG("2");
				/* create thread0 */
				try {
			PDBG("2");
					enum { WEIGHT = Cpu_session::DEFAULT_WEIGHT };
					_thread0_cap = _cpu_session_client.create_thread(WEIGHT, _name);
			PDBG("2");
				} catch (Cpu_session::Thread_creation_failed) {
					PERR("Creation of thread0 failed");
					throw THREAD_FAIL;
				}

				/*
				 * The argument 'elf_ds_cap' may be invalid, which is not an error.
				 * This can happen when the process library is used to set up a process
				 * forked from another. In this case, all process initialization should
				 * be done except for the ELF loading and the startup of the main
				 * thread (as a forked process does not start its execution at the ELF
				 * entrypoint).
				 */
				bool const forked = !elf.valid();

				///* check for dynamic program header */
				//if (!forked && _check_dynamic_elf(elf_ds_cap)) {
				//	if (!_dynamic_linker_cap.valid()) {
				//		PERR("Dynamically linked file found, but no dynamic linker binary present");
				//		throw ELF_FAIL;
				//	}
				//	elf_ds_cap = _dynamic_linker_cap;
				//}

				///* init temporary allocator object */
				//Ram_session_client ram(ram_session_cap);

				/* parse ELF binary and setup segment dataspaces */
				//addr_t entry = 0;
				//if (elf_ds_cap.valid()) {
				//	entry = _setup_elf(parent_cap, elf_ds_cap, ram, _rm_session_client);
				//	_entry = entry;
				//	if (!entry) {
				//		PERR("Setup ELF failed");
				//		throw ELF_FAIL;
				//	}
				//}

				///* register parent interface for new protection domain */
				//if (_pd_session_client.assign_parent(parent_cap)) {
				//	PERR("Could not assign parent interface to new PD");
				//	throw ASSIGN_PARENT_FAIL;
				//}
				PDBG("3");
				/* bind thread0 */
				err = _pd_session_client.bind_thread(_thread0_cap);
				PDBG("3");
				if (err) {
					PERR("Thread binding failed (%d)", err);
					throw THREAD_BIND_FAIL;
				}

				/* register thread0 at region manager session */
				Pager_capability pager;
				try {
				PDBG("4");
					pager = _rm_session_client.add_client(_thread0_cap);
				PDBG("4");
				} catch (...) {
					PERR("Pager setup failed (%d)", err);
					throw THREAD_ADD_FAIL;
				}

				/* set pager in thread0 */
				PDBG("5");
				err = _cpu_session_client.set_pager(_thread0_cap, pager);
				PDBG("5");
				if (err) {
					PERR("Setting pager for thread0 failed");
					throw THREAD_PAGER_FAIL;
				}

				/*
				 * Inhibit start of main thread if the new process happens to be forked
				 * from another. In this case, the main thread will get manually
				 * started after constructing the 'Process'.
				 */
				//if (!forked) {

					/* start main thread */
					PDBG("going to new cpu.start");
					err = _cpu_session_client.start(_thread0_cap, _entry, 0 /* unused */);
					PDBG("new cpu.start");
					if (err) {
						PERR("Thread0 startup failed");
						throw THREAD_START_FAIL;
					}
				//}
			}
			

			catch (Local_exception cause) {

				switch (cause) {

				case THREAD_START_FAIL:
				case THREAD_PAGER_FAIL:
				case THREAD_ADD_FAIL:
				case THREAD_BIND_FAIL:
				case ASSIGN_PARENT_FAIL:
				case ELF_FAIL:

					_cpu_session_client.kill_thread(_thread0_cap);
					_thread0_cap = Thread_capability();

				case THREAD_FAIL:

				default:
					PWRN("unknown exception?");
				}
			}
		}

};

#endif /* _INCLUDE__BASE__PROCESS_H_ */
