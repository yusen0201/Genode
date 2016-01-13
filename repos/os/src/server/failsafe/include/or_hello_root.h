
#include <base/env.h>
#include <base/heap.h>
#include <base/rpc_server.h>
#include <base/rpc_client.h>
#include <root/component.h>
#include <hello_session/hello_session.h>
#include <hello_session/connection.h>
#include <failsafe_session/hello_client.h>

namespace Loader{
	using namespace Genode;

	class Hello_component;
	class Hello_root;
}

using namespace Genode;
class Hello_component : public Rpc_object<Hello::Session>
{
	public:
		 //Hello::Connection con;
	
 		 void say_hello()
                 {
                         PDBG("This is failsafe say()");
		//	 con.say_hello();
                 }
 
                 int add(int a, int b)
                 {
			//return con.add(a, b);
			return a+b;
                 }
		

};

class Hello_root : public Root_component<Hello_component>
{
	private:
	
		Hello_component* _hello;
		//Genode::Lock hello_cap_barrier { Genode::Lock::LOCKED };
		
	protected:	

		Hello_component *_create_session(const char *args)
     		{
        		PDBG("creating hello session to failsafe.");
        		_hello = new (md_alloc()) Hello_component();
			//PDBG("%x", _hello);	
			//hello_cap_barrier.unlock();
			return _hello;
      		}

    	public:

      		Hello_root(Genode::Rpc_entrypoint &ep,
                     	   Genode::Allocator      &allocator)
      		: Genode::Root_component<Hello_component>(&ep, &allocator)
      		{
        		PDBG("Creating root component of failsafe.");
      		}

		//Hello_component* get_component()
		//{
		//	hello_cap_barrier.lock();
		//	return _hello;
		//}
		
};
